#include "epoll_server.h"

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#define MAXEVENTS 4000000

static void die(char* str) {
    perror(str);
    exit(1);
}

static int sock_create_and_bind(char* port) {
    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* result;
    int rc = getaddrinfo(NULL, port, &hints, &result);
    if (rc != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rc));
        return -1;
    }

    struct addrinfo* rp;
    int sock;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sock = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sock == -1)
            continue;

        int one = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

        rc = bind(sock, rp->ai_addr, rp->ai_addrlen);
        if (rc == 0)
            break;

        close(sock);
    }

    if (rp == NULL) {
        fprintf(stderr, "couldn't bind\n");
        return -1;
    }

    freeaddrinfo(result);

    return sock;
}

static int sock_make_nonblocking(int sock) {
    const int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

static int sock_listen(int sock, int maxconn) {
    return listen(sock, maxconn);
}

static void sock_accept(int sock, int epollfd) {
    for (;;) {
        struct sockaddr in_addr;
        socklen_t in_len;

        in_len = sizeof (in_addr);
        const int fd = accept(sock, &in_addr, &in_len);
        if (fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; /* All incoming connections have processed. */
            }
            else {
                perror("accept");
                break;
            }
        }

        char hbuf[NI_MAXHOST];
        char sbuf[NI_MAXSERV];
        int rc = getnameinfo(&in_addr, in_len,
                                   hbuf, sizeof(hbuf),
                                   sbuf, sizeof(sbuf),
                                   NI_NUMERICHOST | NI_NUMERICSERV);
        if (rc == 0)
            printf("fd = %d (host = %s, port = %s)\n",
                   fd, hbuf, sbuf);

        rc = sock_make_nonblocking(fd);
        if (rc == -1) {
            fprintf(stderr, "can't make sock nonblocking");
            close(fd);
            break;
        }

        struct epoll_event event;
        event.data.fd = fd;
        event.events = EPOLLIN | EPOLLET;

        rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
        if (rc == -1) {
            perror("epoll_ctl");
            close(fd);
            break;
        }
    }
}

int run_server(char* port,
               void (*read_cb_handler) (struct context* context),
               void (*write_cb_handler) (struct context* context),
               void (*err_cb_handler) (struct context* context)) {
    const int sock = sock_create_and_bind(port);
    if (sock == -1)
        die("create and bind socket");

    int rc = sock_make_nonblocking(sock);
    if (rc == -1)
        die("make socket nonblocking");

    rc = sock_listen(sock, SOMAXCONN);
    if (rc == -1)
        die("listen");

    const int epollfd = epoll_create1(0);
    if (epollfd == -1)
        die("epoll_create1");

    struct epoll_event event;
    memset(&event, 0, sizeof(struct epoll_event));
    event.data.fd = sock;
    event.events = EPOLLET;
    if (read_cb_handler)
        event.events |= EPOLLIN;
    if (write_cb_handler)
        event.events |= EPOLLOUT;
    if (err_cb_handler)
        event.events |= EPOLLERR | EPOLLHUP;

    rc = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock, &event);
    if (rc == -1)
        die("epoll_ctl");

    struct epoll_event* events =
        (struct epoll_event*) calloc(MAXEVENTS, sizeof(event));

    for (;;) {
        const int n = epoll_wait(epollfd, events, MAXEVENTS, -1);
        int i;
        for (i = 0; i < n; ++i) {
            struct context context = {
                .event = &events[i],
                .epollfd = epollfd,
                .data = NULL
            };

            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP)) {
                if (err_cb_handler) {
                    err_cb_handler(&context);
                }
                else {
                    fprintf(stderr, "epoll error\n");
                    close(events[i].data.fd);
                }
                continue;
            }

            if (sock == events[i].data.fd) {
                sock_accept(sock, epollfd);
                continue;
            }

            if (events[i].events & EPOLLOUT) {
                if (write_cb_handler)
                    write_cb_handler(&context);
            }

            if (events[i].events & EPOLLIN) {
                if (read_cb_handler)
                    read_cb_handler(&context);
            }
        }
    }

    free(events);
    close(sock);
}
