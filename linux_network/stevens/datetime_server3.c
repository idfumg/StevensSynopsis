#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

/*
  Если необходимо отправить сегмент FIN по TCP соединению, то
  вместо функции close должна использоваться функция shutdown.
 */

int server_prepare() {
    printf("%s\n", __FUNCTION__);
    const int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(1234);

    bind(listenfd, (struct sockaddr*) &serveraddr, sizeof(serveraddr));
    listen(listenfd, BUFSIZ);

    return listenfd;
}

void server_action(const int connfd, struct sockaddr_in* clientaddr) {
    char buff[BUFSIZ] = {0};
    time_t ticks;

    inet_ntop(AF_INET, &clientaddr->sin_addr, buff, sizeof(buff));
    printf("[%d] connection from %s, port %d\n",
           (int) getpid(), buff, ntohs(clientaddr->sin_port));

    ticks = time(NULL);
    snprintf(buff, sizeof(buff), "%24s\n", ctime(&ticks));
    write(connfd, buff, strlen(buff));

    close(connfd);
}

int main() {
    const int listenfd = server_prepare();

    int connfd;
    struct sockaddr_in clientaddr;
    socklen_t len;
    pid_t pid;

    for (;;) {
        len = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr*) &clientaddr, &len);

        if ((pid = fork()) == 0) {
            close(listenfd);
            server_action(connfd, &clientaddr);
            exit(0);
        }

        close(connfd);
    }

    close(listenfd);

    return 0;
}
