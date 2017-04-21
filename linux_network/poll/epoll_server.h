#ifndef EPOLL_SERVER_H__
#define EPOLL_SERVER_H__

#include <sys/epoll.h>

struct context {
    struct epoll_event* event;
    int epollfd;
    void* data;
};

int run_server(char* port,
               void (*read_cb_handler) (struct context* context),
               void (*write_cb_handler) (struct context* context),
               void (*err_cb_handler) (struct context* context));

#endif /* EPOLL_SERVER_H__ */
