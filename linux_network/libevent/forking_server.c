#include <netinet/in.h>
#include <sys/socket.h>

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 6666

void child(int fd) {
    char buf[BUFSIZ];
    ssize_t nread;
    ssize_t nwritten;

    for (;;) {
        nread = recv(fd, buf, sizeof(buf), 0);

        if (nread == 0) {
            break;
        }
        else if (nread < 0) {
            perror("read");
            close(fd);
            return;
        }

        nwritten = send(fd, buf, nread, 0);

        if (nwritten <= 0) {
            perror("send");
            return;
        }
    }

    close(fd);
}

void run(void) {
    const int listener = socket(AF_INET, SOCK_STREAM, 0);

    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(PORT);
    sin.sin_addr.s_addr = INADDR_ANY;

    if (bind(listener, (struct sockaddr*) &sin, sizeof(sin)) < 0) {
        perror("bind");
        return;
    }

    if (listen(listener, 16) < 0) {
        perror("listen");
        return;
    }

    for (;;) {
        struct sockaddr_storage ss;
        socklen_t slen = sizeof(ss);

        const int fd = accept(listener, (struct sockaddr*) &ss, &slen);

        if (fd < 0) {
            perror("accept");
        }
        else {
            if (fork() == 0) {
                child(fd);
                exit(0);
            }
        }
    }
}

int main(int argc, char** argv) {
    run();
    return 0;
}
