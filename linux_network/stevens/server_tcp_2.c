#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#define SERVPORT 6666
#define LISTENQ BUFSIZ

int to_daemon();

int main() {
    to_daemon();

    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVPORT);
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);

    int maxfd = listenfd;
    int client[FD_SETSIZE];
    int client_index = -1;
    int i;

    for (i = 0; i < sizeof(client) / sizeof(client[0]); ++i)
        client[i] = -1;

    fd_set rset, allset;
    FD_ZERO(&allset);
    FD_SET(listenfd, &allset);

    int nready;
    socklen_t clientaddr_len;
    struct sockaddr_in clientaddr;
    int connfd, sockfd;
    int n;
    char buff[BUFSIZ];

    for (;;) {
        rset = allset;
        nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

        if (FD_ISSET(listenfd, &rset)) {
            clientaddr_len = sizeof(clientaddr);
            connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &clientaddr_len);

            for (i = 0; i < sizeof(client) / sizeof(client[0]); ++i)
                if (client[i] < 0) {
                    client[i] = connfd;
                    break;
                }

            if (i == sizeof(client) / sizeof(client[0])) {
                printf("too many clients\n");
                exit(1);
            }

            FD_SET(connfd, &allset);
            if (connfd > maxfd)
                maxfd = connfd;
            if (i > client_index)
                client_index = i;

            if (--nready <= 0)
                continue;
        }

        for (i = 0; i <= client_index; ++i) {
            if ((sockfd = client[i]) < 0)
                continue;
            if (!FD_ISSET(sockfd, &rset))
                continue;
            if ((n = read(sockfd, buff, sizeof(buff))) == 0) {
                close(sockfd);
                FD_CLR(sockfd, &allset);
                client[i] = -1;
            } else {
                write(sockfd, buff, n);
            }
            if (--nready <= 0)
                break;
        }
    }

    return 0;
}
