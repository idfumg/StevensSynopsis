#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/select.h>
#include <fcntl.h>

#define SERVPORT 6666
#define CONNS 20

int max(int x, int y) {
    return x > y ? x : y;
}

void handle_request(FILE* fp, const int sockfd) {
    char buff[BUFSIZ] = {0};
    ssize_t n;
    fd_set rset;
    int maxfd;
    int stdineof = 0;

    FD_ZERO(&rset);
    for (;;) {
        if (stdineof == 0)
            FD_SET(fileno(fp), &rset);
        FD_SET(sockfd, &rset);
        maxfd = max(fileno(fp), sockfd) + 1;
        select(maxfd, &rset, NULL, NULL, NULL);

        if (FD_ISSET(sockfd, &rset)) {
            if ((n = read(sockfd, buff, sizeof(buff))) <= 0) {
                if (stdineof == 1)
                    return;

                perror("server terminated prematurely");
                exit(1);
            }
            write(fileno(stdout), buff, n);
        }

        if (FD_ISSET(fileno(fp), &rset)) {
            if ((n = read(fileno(fp), buff, sizeof(buff))) <= 0) {
                stdineof = 1;
                shutdown(sockfd, SHUT_WR); /* send FIN segment */
                FD_CLR(fileno(fp), &rset);
                continue;
            }
            write(sockfd, buff, n);
        }
    }
}

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s <IPaddress>", argv[0]);
        exit(1);
    }

    int sockfd[CONNS];
    int i;

    for (i = 0; i < CONNS; ++i) {
        sockfd[i] = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in servaddr;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(SERVPORT);
        inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr);

        connect(sockfd[i], (struct sockaddr*) &servaddr, sizeof(servaddr));
        fcntl_setflag(sockfd[i], O_NONBLOCK);
    }

    handle_request(stdin, sockfd[0]);

    return 0;
}
