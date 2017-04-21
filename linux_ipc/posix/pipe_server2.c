#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SERVER_FIFO "/tmp/fifo.server"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    if (mkfifo(SERVER_FIFO, FILE_MODE) == -1 && errno != EEXIST) {
        perror("can't create fifo file");
        exit(1);
    }

    int readfd = open(SERVER_FIFO, O_RDONLY, 0);
    int dummyfd = open(SERVER_FIFO, O_WRONLY, 0); // don't used

    ssize_t nbytes;
    char buff[BUFSIZ + 1];

    for (;;) {
        nbytes = read(readfd, buff, sizeof (buff));

        if (nbytes > 0)
            break;
    }

    buff[nbytes] = '\0';

    char* ptr = strchr(buff, ' ');

    if (ptr == NULL) {
        printf("wrong request: '%s'\n", *ptr);
        exit(1);
    }

    *ptr++ = '\0';

    char fifoname[BUFSIZ];
    char filename[BUFSIZ];
    const pid_t pid = atol(buff);
    snprintf(fifoname, sizeof (fifoname), "/tmp/fifo.%ld", (long) pid);
    snprintf(filename, sizeof (filename), "%s", ptr);

    int writefifo = open(fifoname, O_WRONLY, 0);
    int filefd = open(filename, O_RDONLY, 0);

    if (filefd == -1) {
        snprintf(filename + strlen(ptr), sizeof (filename) - strlen(ptr),
                 ": can't open: %s", strerror(errno));
        write(writefifo, filename, strlen (filename));

        exit(1);
    }

    while ((nbytes = read(filefd, buff, sizeof (buff))) > 0)
        write(writefifo, buff, nbytes);

    return 0;
}
