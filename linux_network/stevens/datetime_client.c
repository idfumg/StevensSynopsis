#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#define PORT 1234
#define IPADDR "127.0.0.1"

/*
  htons - host to network short.
  ntohs - network to host short.
  htonl - host to network long.
  pton - presentation to numeric.

  Функции, которые работают с IPv4:
  inet_aton - строку в 32битное число,
  inet_ntoa - число в строку,
  inet_addr - строку в число в сетевом порядке байтов.
  Примечание: inet_addr не работает с 255.255.255.255.

  Новые функции для работы как с IPv4, так и с IPv6:
  inet_pton - строку в число,
  inet_ntop - число в строку.

  Для работы с ними предоставляются две константы:
  #define INET_ADDRSTRLEN 16
  #define INET6_ADDRSTRLEN 46

 */
int main() {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IPADDR);
//    inet_pton(AF_INET, IPADDR, &servaddr.sin_addr);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket creation error");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr)) == -1) {
        perror("socket connect error");
        exit(1);
    }

    char buff[BUFSIZ] = {0};
    int nbytes = -1;
    while ((nbytes = read(sockfd, buff, BUFSIZ)) > 0) {
        buff[nbytes] = 0;
        fputs(buff, stdout);
    }

    if (nbytes < 0) {
        perror("socket read error");
        exit(1);
    }

    return 0;
}
