#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/select.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define PORT 6666

struct fd_set {
    char buffer[BUFSIZ];
    int need_write;
};

struct fd_set* alloc_fd_set(void) {
    struct fd_set* state = malloc(sizeof(struct fd_set));
    if (!state)
        return NULL;

    memset(state, 0, sizeof(struct fd_set));

    return state;
}

void free_fd_set(struct fd_set* state) {
    free(state);
}

void make_nonblocking(int fd) {
    fcntl(fd, F_SETFL, O_NONBLOCK);
}

int do_read(int fd, struct fd_set* state) {
    ssize_t nread;
    char* cp = state->buffer;

    for (;;) {
        nread = recv(fd,
                     cp,
                     sizeof(state->buffer) - sizeof(cp),
                     0);

        if (nread == 0) {
            break;
        }
        else if (nread < 0) {
            if (errno == EAGAIN)
                break;
            perror("\nrecv");
            return -1;
        }

        state->need_write = 1;
        cp += nread;
    }

    return 0;
}

int do_write(int fd, struct fd_set* state) {
    ssize_t remaining = strlen(state->buffer);
    ssize_t nwritten;
    const char* cp = state->buffer;

    printf("I'd want to write: '%s' to client\n", state->buffer);

    while (remaining) {
        nwritten = send(fd, cp, remaining, 0);

        if (nwritten <= 0) {
            perror("send");
            return -1;
        }

        remaining -= nwritten;
        cp += nwritten;
    }

    return -1;
}

void run() {
    const int listener = socket(AF_INET, SOCK_STREAM, 0);
    make_nonblocking(listener);
    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

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

    struct fd_set* state[FD_SETSIZE];
    int i;
    for (i = 0; i < FD_SETSIZE; ++i)
        state[i] = NULL;

    fd_set readset, writeset, exset;
    int maxfd;

    for (;;) {
        maxfd = listener;
        FD_ZERO(&readset);
        FD_ZERO(&writeset);
        FD_ZERO(&exset);

        FD_SET(listener, &readset);


        for (i = 0; i < FD_SETSIZE; ++i) {
            if (state[i]) {
                if (i > maxfd)
                    maxfd = i;
                FD_SET(i, &readset);
                if (state[i]->need_write)
                    FD_SET(i, &writeset);
            }
        }

        if (select(maxfd + 1,
                   &readset,
                   &writeset,
                   &exset,
                   NULL) < 0) {
            perror("select");
            return;
        }

        if (FD_ISSET(listener, &readset)) {
            struct sockaddr_storage ss;
            socklen_t slen = sizeof(ss);
            const int fd = accept(listener,
                                  (struct sockaddr*) &ss, &slen);
            if (fd < 0) {
                perror("accept");
            }
            else if (fd > FD_SETSIZE) {
                close(fd);
            }
            else {
                make_nonblocking(fd);
                state[fd] = alloc_fd_set();
                assert(state[fd]);
            }
        }

        for (i = 0; i < maxfd + 1; ++i) {
            int r = 0;
            if (i == listener)
                continue;
            if (FD_ISSET(i, &readset))
                r = do_read(i, state[i]);
            if (r == 0 && FD_ISSET(i, &writeset))
                r = do_write(i, state[i]);
            if (r == -1) {
                free_fd_set(state[i]);
                state[i] = NULL;
                close(i);
            }
        }
    }
}

int main(int argc, char** argv) {
    setvbuf(stdout, NULL, _IONBF, 0);

    run();
    return 0;
}
