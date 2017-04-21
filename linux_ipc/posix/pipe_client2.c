#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#define SERVER_FIFO "/tmp/fifo.server"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    int writefd, readfd;
    char buff[BUFSIZ];
    size_t nbytes;

    pid_t pid = getpid();

    writefd = open(SERVER_FIFO, O_WRONLY, 0);

    if (writefd == -1) {
        perror("wrong server fifo name");
        exit(1);
    }

    sprintf(buff, "%ld %s", (long) pid, "pipe4-server.c");
    write(writefd, buff, strlen (buff));

    sprintf(buff, "/tmp/fifo.%ld", pid);

    if (mkfifo(buff, FILE_MODE) == -1 && errno != EEXIST) {
        perror("create pipe fifo error");
        exit(1);
    }

    readfd = open(buff, O_RDONLY, 0);

    if (readfd == -1) {
        perror("can't open client pipe fifo");
        exit(1);
    }

    while ((nbytes = read(readfd, buff, sizeof(buff))) > 0)
        write(STDOUT_FILENO, buff, nbytes);

    return 0;
}
