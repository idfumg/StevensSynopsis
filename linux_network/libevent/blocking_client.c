#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char** argv) {
    const char query[] =
        "GET / HTTP/1.0\r\n"
        "Host: www.google.ru\r\n"
        "\r\n";
    const char hostname[] = "www.google.ru";

    struct hostent* host = gethostbyname(hostname);

    if (!host) {
        fprintf(stderr, "couldn't lookup %s: %s", hostname, hstrerror(h_errno));
        return 1;
    }

    if (host->h_addrtype != AF_INET) {
        fprintf(stderr, "no ipv6 support, sorry");
        return 1;
    }

    const int fd = socket(AF_INET, SOCK_STREAM, 0);

    if (fd < 0) {
        perror("socket");
        return 1;
    }

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(80);
    sin.sin_addr = *(struct in_addr*) host->h_addr_list[0];

    if (connect(fd, (struct sockaddr*) &sin, sizeof(sin))) {
        perror("connect");
        close(fd);
        return 1;
    }

    const char* cp = query;
    ssize_t remaining = strlen(query);
    ssize_t nwritten;

    while (remaining) {
        nwritten = send(fd, cp, remaining, 0);

        if (nwritten <= 0) {
            perror("send");
            return 1;
        }

        remaining -= nwritten;
        cp += nwritten;
    }

    char buf[BUFSIZ];

    for (;;) {
        const ssize_t result = recv(fd, buf, sizeof(buf), 0);

        if (result == 0) {
            break;
        }
        else if (result < 0) {
            perror("recv");
            close(fd);
            return 1;
        }

        fwrite(buf, 1, result, stdout);
    }

    close(fd);
    return 0;
}
