/*
  Используется механизм именнованных каналов.
  Данные каналы создаются на уровне файловой системы.
  Если FIFO уже существует, происходит ошибка EEXIST.
  Созданные каналы открываются командой open.
  При вызове lseek возвращается ошибка ESPIPE.
  Преимущество именнованных каналов в том, что использовать их могут
  неродственные процессы.
  Минус в том, что необходимо реализовать протокол передачи данных,
  поскольку считывание данных происходит побайтно.
  Именно клиент удаляет канал FIFO, поскольку последние проводимые операции
  с каналом выполняются именно им.
  Ядром гарантируется атомарность операции записи в канал, если
  размер данных не превышает PIPE_BUF.
*/

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FIFO1 "/tmp/fifo1"
#define FIFO2 "/tmp/fifo2"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void server(int readfd, int writefd) {
	char buff[BUFSIZ];
    size_t nbytes = read(readfd, buff, sizeof (buff));

	if (nbytes == 0) {
		perror("end of line when reading file name");
		exit(1);
	}

	buff[nbytes] = '\0';

	int fd = open(buff, O_RDONLY);

	if (fd < 0) {
		snprintf(buff + nbytes, sizeof (buff) - nbytes,
				 ": can't open file: %s", strerror(errno));
		write(writefd, buff, strlen(buff));

		return;
	}

	while ((nbytes = read(fd, buff, sizeof (buff))) > 0)
		write(writefd, buff, nbytes);

	close(fd);
}

void client(int readfd, int writefd) {
	char buff[BUFSIZ] = "pipe_fifo.c";
	size_t nbytes;

	write(writefd, buff, strlen(buff));

	while ((nbytes = read(readfd, buff, BUFSIZ)) > 0)
		write(STDOUT_FILENO, buff, nbytes);
}

int main() {
	int writefd;
    int readfd;

	if (mkfifo(FIFO1, FILE_MODE) == -1 && errno != EEXIST) {
		perror("can't create fifo file");
		exit(1);
	}

	if (mkfifo(FIFO2, FILE_MODE) == -1 && errno != EEXIST) {
		unlink(FIFO1);
		perror("can't create fifo file");
		exit(1);
	}

	const pid_t childpid = fork();

	if (childpid == 0) {
		readfd = open(FIFO1, O_RDONLY, 0);
		writefd = open(FIFO2, O_WRONLY, 0);
		server(readfd, writefd);

		close(readfd);
		close(writefd);

		exit(0);
    }

    writefd = open(FIFO1, O_WRONLY, 0);
    readfd = open(FIFO2, O_RDONLY, 0);
    client(readfd, writefd);
    waitpid(childpid, NULL, 0);

	close(readfd);
	close(writefd);

	unlink(FIFO1);
	unlink(FIFO2);

	return 0;
}
