#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <event2/event.h>

#define PORT 6666

void do_read(evutil_socket_t fd, short events, void* arg);
void do_write(evutil_socket_t fd, short events, void* arg);

struct fd_state {
    char buffer[BUFSIZ];

    struct event* read_event;
    struct event* write_event;
};

void free_fd_state(struct fd_state* state) {
    event_free(state->read_event);
    event_free(state->write_event);
    free(state);
}

struct fd_state* alloc_fd_state(struct event_base* base,
                                evutil_socket_t fd) {
    struct fd_state* state = malloc(sizeof(struct fd_state));

    if (!state)
        return NULL;

    memset(state, 0, sizeof(struct fd_state));

    state->read_event = event_new(base, fd,
                                  EV_READ | EV_PERSIST,
                                  do_read, state);
    if (!state->read_event) {
        free(state);
        return NULL;
    }

    state->write_event = event_new(base, fd,
                                   EV_WRITE | EV_PERSIST,
                                   do_write,
                                   state);
    if (!state->write_event) {
        event_free(state->read_event);
        free(state);
        return NULL;
    }

    return state;
}

void do_read(evutil_socket_t fd, short events, void* arg) {
    struct fd_state* state = arg;
    ssize_t nread;
    char* cp = state->buffer;

    for (;;) {
        nread = recv(fd,
                     cp,
                     sizeof(state->buffer) - strlen(state->buffer),
                     0);

        if (nread <= 0)
            break;

        cp += nread;
    }

    if (nread == 0) {
        free_fd_state(state);
    }
    else if (nread < 0) {
        if (errno == EAGAIN) {
            event_add(state->write_event, NULL);
            return;
        }
        perror("recv");
        free_fd_state(state);
    }
}

void do_write(evutil_socket_t fd, short events, void* arg) {
    struct fd_state* state = arg;
    ssize_t nwritten;
    ssize_t remaining = strlen(state->buffer);
    const char* cp = state->buffer;

    while (remaining) {
        nwritten = send(fd, cp, remaining, 0);

        if (nwritten <= 0) {
            if (errno == EAGAIN)
                break;

            perror("send");
            free_fd_state(state);
        }

        remaining -= nwritten;
        cp += nwritten;
    }

    event_del(state->write_event);
}

void do_accept(evutil_socket_t listener,
               short event,
               void* arg) {
    struct event_base* base = arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);

    const int fd = accept(listener, (struct sockaddr*) &ss, &slen);

    if (fd < 0) {
        perror("accept");
    }
    else if (fd > FD_SETSIZE) {
        close(fd);
    }
    else {
        evutil_make_socket_nonblocking(fd);
        struct fd_state* state = alloc_fd_state(base, fd);
        assert(state);
        event_add(state->read_event, NULL);
    }
}

void run() {
    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
               &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = 0;

    if (bind(listener, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    struct event_base* base = event_base_new();

    if (!base)
        return;

    //printf("current methods is: %s\n", event_base_get_method(base));

    evutil_make_socket_nonblocking(listener);

    struct event* listener_event = event_new(base,
                                             listener,
                                             EV_READ | EV_PERSIST,
                                             do_accept,
                                             (void*) base);
    event_add(listener_event, NULL);
    event_base_dispatch(base);
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);
    run();

    return 0;
}
