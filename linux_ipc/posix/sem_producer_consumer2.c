/*
  Использование семафоров. Неименованные семафоры.
  Может не представлять собой объект файловой системы.
  Используется, если необходимо разместить семафор
  в разделяемой памяти между несколькими процессами.
  Для его создания, необходимо вызвать функцию
  инициализации sem_init.
  Для его удаления - sem_destroy.
  Для использования неименнованного семафора
  несколькими процессами, необходимо использовать
  при его инициализации одно и тоже имя.
  Если не нужен доступ из файловой системы, создание
  такого семафора выглядит более компактным.
  При завершении процесса, если семафор не был размещен
  в разделяемой памяти, он будет уничтожен.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define NITEMS 1000000
#define NBUFF 10

struct {
    int buff[NBUFF];
    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
} shared = { {0}, 0, 0, 0 };

void* produce(void* arg) {
    int i;

    for (i = 0; i < NITEMS; ++i) {
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);
        shared.buff[i % NBUFF] = i;
        sem_post(&shared.mutex);
        sem_post(&shared.nstored);
    }
}

void* consume(void* arg) {
    int i;

    for (i = 0; i < NITEMS; ++i) {
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);
        //printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);
    }
}

int main() {
    pthread_t producer;
    pthread_t consumer;

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

    return 0;
}
