/*
  Создаются два канала для общения между клиентом и сервером,
  посредством порождения процесса.

  Клиент пишет в канал название файла.
  Затем, читает из канала переданный файл.
  Операция чтения - блокирующая и контролируется ядром.
  Операция чтения вернет 0, если был получен признак конца файла.

  Сервер читает из канала имя файла.
  Затем, открывает файл на чтение, читает содержимое и отправляет клиенту.
  В случае ошибки отправляет клиенту через канал сообщение об ошибке.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

enum PIPE_VALUES {
    PIPE_READ = 0,
    PIPE_WRITE = 1,
    PIPE_COUNT = 2
};

void server(int readfd, int writefd) {
    int fd;
    ssize_t nbytes;
    char buff[BUFSIZ];

    nbytes = read(readfd, buff, BUFSIZ);

    if (nbytes == 0) {
        perror("end-of-file while reading pathname");
        exit(1);
    }

    buff[nbytes] = '\0';

    fd = open(buff, O_RDONLY);

    if (fd < 0) {
        snprintf(buff + nbytes, sizeof (buff) - nbytes,
                 ": cant open, %s\n", strerror(errno));
        nbytes = strlen(buff);
        write(writefd, buff, nbytes);

        return;
    }

    while ((nbytes = read(fd, buff, BUFSIZ)) > 0)
        write(writefd, buff, nbytes);

    close(fd);
}

void client(int readfd, int writefd) {
    ssize_t nbytes;
    char buff[BUFSIZ];

    sprintf(buff, "pipe_client_server1.c");
    write(writefd, buff, strlen(buff));

    while ((nbytes = read(readfd, buff, BUFSIZ)) > 0)
        write(STDOUT_FILENO, buff, nbytes);
}

int main(void) {
    int fd1[PIPE_COUNT];
    int fd2[PIPE_COUNT];
    pid_t childpid;

    pipe(fd1);
    pipe(fd2);

    if((childpid = fork()) == 0) {
        close(fd1[PIPE_WRITE]);
        close(fd2[PIPE_READ]);

        server(fd1[PIPE_READ], fd2[PIPE_WRITE]);

        exit(0);
    }

    close(fd1[PIPE_READ]);
    close(fd2[PIPE_WRITE]);

    client(fd2[PIPE_READ], fd1[PIPE_WRITE]);
    waitpid(childpid, NULL, 0);

    return 0;
}
