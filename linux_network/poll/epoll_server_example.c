#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "epoll_server.h"

static void read_cb(struct context* context) {
    struct epoll_event* event = context->event;
    const int fd = event->data.fd;
    ssize_t count;
    char buf[BUFSIZ];
    int rc;

    for (;;) {
        count = read(fd, buf, sizeof(buf));
        if (count == -1) {
            if (errno == EINTR || errno == EAGAIN)
                break;
            perror("read");
            break;
        }
        else if (count == 0) {
            break;
        }

        rc = write(1, buf, count);
        if (rc == -1) {
            perror("write");
            break;
        }
    }
    printf("\n");

    event->events = EPOLLOUT | EPOLLET;

    rc = epoll_ctl(4, EPOLL_CTL_MOD, event->data.fd, event);
    if (rc == -1)
        perror("epoll_ctl");
}

static void write_cb(struct context* context) {
    struct epoll_event* event = context->event;
    char buf[BUFSIZ] = "answer to client:)";
    int rc = write(event->data.fd, buf, strlen(buf));
    if (rc <= 0) {
        perror("write");
    }
    close(event->data.fd);

    /* rc = epoll_ctl(4, EPOLL_CTL_DEL, event->data.fd, event); */
    /* if (rc == -1) */
    /*     perror("epoll_ctl"); */
}

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s [port]\n", argv[0]);
        return 1;
    }

    run_server(argv[1], read_cb, write_cb, NULL);

    return 0;
}
