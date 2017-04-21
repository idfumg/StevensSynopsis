/*
  Использование семафоров.
  Реализована схема, при которой один производитель
  и один потребитель для взаимодействия используют
  несколько буферов для чтения данных из файла.
  Таким образом, мы можем выиграть время, которое
  уходит на ожидание, когда потребитель заберет
  данные из буфера, если он представлен в
  единичном экземпляре.
  Производитель по завершении работы предоставляет
  возможность потребителю закончить свою работу.
  Блокирование происходит только в маленьком участке кода.
  Блокировать запись в буфер не нужно, поскольку
  запись производится всегда в отдельный буфер массива.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFFSIZE 4000
#define NBUFF 10
#define FILENAME "sem_producer_consumer5.c"

struct {
    struct {
        char data[BUFFSIZE];
        ssize_t n;
    } buff[NBUFF];

    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
} shared = { 0, 0, 0 };

int fd;

void* produce(void* arg) {
    int i = 0;

    for (;;) {
        sem_wait(&shared.nempty);

        sem_wait(&shared.mutex);
        /* critical region */
        sem_post(&shared.mutex);

        shared.buff[i].n = read(fd, shared.buff[i].data,
                                sizeof(shared.buff[i].data));
        if (shared.buff[i].n == 0) {
            sem_post(&shared.nstored);
            return NULL;
        }

        if (++i >= NBUFF)
            i = 0;

        sem_post(&shared.nstored);
    }
}

void* consume(void* arg) {
    int i = 0;

    for (;;) {
        sem_wait(&shared.nstored);

        sem_wait(&shared.mutex);
        /* critical region */
        sem_post(&shared.mutex);

        if (shared.buff[i].n == 0)
            return NULL;

        write(STDOUT_FILENO, shared.buff[i].data, shared.buff[i].n);

        if (++i >= NBUFF)
            i = 0;

        sem_post(&shared.nempty);
    }
}

int main() {
    pthread_t producer;
    pthread_t consumer;

    fd = open(FILENAME, O_RDONLY);

    sem_init(&shared.mutex, 0, 1);
    sem_init(&shared.nempty, 0, NBUFF);
    sem_init(&shared.nstored, 0, 0);

    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    sem_destroy(&shared.mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nstored);

    close(fd);

    return 0;
}
