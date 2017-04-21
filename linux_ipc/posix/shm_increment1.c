#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SEM_NAME "mysem"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define NLOOP 100

int count = 0;

/*
  У каждого процесса свои копии данных.
  Открытый дескриптор семафора может использоваться
  обоими процессами.
*/
int main() {
    int i;
    sem_t* mutex;

    mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, FILE_MODE, 1);
    sem_unlink(SEM_NAME); /* удаляется из системы, но пока открыт */

    setbuf(stdout, NULL); /* stdout не буферизуется */

    if (fork() == 0) {
        for (i = 0; i < NLOOP; ++i) {
            sem_wait(mutex);
            printf("child: %d\n", count++);
            sem_post(mutex);
        }
        exit(0);
    }

    for (i = 0; i < NLOOP; ++i) {
        sem_wait(mutex);
        printf("parent: %d\n", count++);
        sem_post(mutex);
    }

    return 0;
}
