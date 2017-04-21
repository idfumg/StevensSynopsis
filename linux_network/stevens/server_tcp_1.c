#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define SERVPORT 6666
#define LISTENQ BUFSIZ

typedef void (*sigfunc)(int signo);

sigfunc Signal(int signo, sigfunc fn) {
    struct sigaction act, oact/* Old signal action. */;

    act.sa_handler = fn;
    sigemptyset(&act.sa_mask);
    if (signo != SIGALRM) { /* We want interrupt system call when io. */
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0) {
        perror("sigaction error");
        return SIG_ERR;
    }

    return oact.sa_handler;
}

void signal_handler(int signo) {
    int stat;
    pid_t pid;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("child %d terminated, state %d\n", pid, stat);
}

void handle_request(const int connfd) {
    ssize_t n;
    char buff[BUFSIZ];

    for (;;) {
        if ((n = read(connfd, buff, sizeof(buff))) <= 0)
            return;
        write(connfd, buff, n);
    }
}

int main() {
    Signal(SIGCHLD, signal_handler);

    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    struct sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof (serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERVPORT);

    bind(listenfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    listen(listenfd, LISTENQ);

    struct sockaddr_in client_addr;
    socklen_t client_len;
    int connfd;
    pid_t childpid;

    for (;;) {
        client_len = sizeof(client_addr);
        if ((connfd = accept(listenfd, (struct sockaddr*) &client_addr, &client_len)) < 0) {
            if (errno = EINTR) {
                printf("accept error (errno = EINTR)");
                continue;
            } else {
                perror("accept error");
            }
        }

        if ((childpid = fork()) == 0) {
            close(listenfd);
            handle_request(connfd);
            exit(0);
        }
        close(connfd);
    }

    return 0;
}
