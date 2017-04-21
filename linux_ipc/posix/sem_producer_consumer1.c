/*
  Использование семафоров. Именованные семафоры.
  Могут реализовываться с помощью файловой системы.
  Благодаря именнованным семафорам неродственные
  процессы могу получить доступ к блокировке.
  Может использоваться как при синхронизации
  родственных/неродственных процессов, так и
  потоков одного процесса/потоков разных процессов.
  Семафоры обладают живучестью файловой системы,
  поэтому удалить семафор можно только с помощью
  функции sem_unlink.
  Ядро не управляет автоматическим увеличением
  счетчика/удалением семафора. При аварийном
  завершении процесса с помощью  сигнала, pthread_cancel,
  exit, заблокированный семафор останется заблокированным.
  Для автоматической разблокировки ядром используется
  блокировка записей и файлов.
  Семафоры хорошо применимы для реализации многопоточного
  доступа к буферу ограниченного размера,в который
  пишется большое количество данных.
  Двоичный семафор работает также, как и взаимное исключение.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SEM_MUTEX "mutex"
#define SEM_NEMPTY "nempty"
#define SEM_NSTORED "nstored"
#define NITEMS 1000000
#define NBUFF 10

#define SEM_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct {
    int buff[NBUFF];
    sem_t* mutex;
    sem_t* nempty;
    sem_t* nstored;
} shared = { {0}, 0, 0, 0 };

void* produce(void* arg) {
    int i;

    for (i = 0; i < NITEMS; ++i) {
        sem_wait(shared.nempty);
        sem_wait(shared.mutex);
        shared.buff[i % NBUFF] = i;
        sem_post(shared.mutex);
        sem_post(shared.nstored);
    }
}

void* consume(void* arg) {
    int i;

    for (i = 0; i < NITEMS; ++i) {
        sem_wait(shared.nstored);
        sem_wait(shared.mutex);
        //printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        sem_post(shared.mutex);
        sem_post(shared.nempty);
    }
}

int main() {
    pthread_t producer;
    pthread_t consumer;

    shared.mutex = sem_open(SEM_MUTEX, SEM_FLAGS, FILE_MODE, 1);
    shared.nempty = sem_open(SEM_NEMPTY, SEM_FLAGS, FILE_MODE, NBUFF);
    shared.nstored = sem_open(SEM_NSTORED, SEM_FLAGS, FILE_MODE, 0);

    pthread_create(&producer, NULL, produce, NULL);
    pthread_create(&consumer, NULL, consume, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, NULL);

    sem_close(shared.mutex);
    sem_close(shared.nempty);
    sem_close(shared.nstored);

    sem_unlink(SEM_MUTEX);
    sem_unlink(SEM_NEMPTY);
    sem_unlink(SEM_NSTORED);

    return 0;
}
