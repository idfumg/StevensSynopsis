#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define MESGSIZE 256
#define NMESG 16

#define SHMNAME "myshm"
#define SEMNAME "mysem"

#define FILE_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

/*
  При работе с разделяемой памятью, нельзя использовать
  указатели, т.к. возвращаемое mmap значение для каждого процесса
  может быть индивидуальным (объект разделяемой памяти
  может быть отображен в разные области адресного
  пространства процессов).
  Поэтому, массив смещений сообщений и массив сообщений
  используются вместе.
 */

struct shmstruct {
    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
    int nput;
    long noverflow;
    sem_t noverflow_mutex;
    long msgoff[NMESG];
    char msgdata[NMESG * MESGSIZE];
};

int main() {
    shm_unlink(SHMNAME);
    int fd = shm_open(SHMNAME, FILE_FLAGS, FILE_MODE);
    struct shmstruct* shmstruct_ptr = mmap(NULL, sizeof (struct shmstruct),
                                           PROT_READ | PROT_WRITE,
                                           MAP_SHARED, fd, 0);
    ftruncate(fd, sizeof (struct shmstruct));
    close(fd);

    int i;
    for (i = 0; i < NMESG; ++i)
        shmstruct_ptr->msgoff[i] = i * MESGSIZE;

    sem_init(&shmstruct_ptr->mutex, 1, 1);
    sem_init(&shmstruct_ptr->nempty, 1, NMESG);
    sem_init(&shmstruct_ptr->nstored, 1, 0);
    sem_init(&shmstruct_ptr->noverflow_mutex, 1, 1);

    int offset;
    int index = 0;
    long noverflow_last = 0;
    long noverflow_tmp = 0;

    for (;;) {
        sem_wait(&shmstruct_ptr->nstored);
        sem_wait(&shmstruct_ptr->mutex);

        offset = shmstruct_ptr->msgoff[index];
        printf("msgdata[%d] = %s\n", offset, &shmstruct_ptr->msgdata[offset]);

        if (++index >= NMESG)
            index = 0;

        sem_post(&shmstruct_ptr->mutex);
        sem_post(&shmstruct_ptr->nempty);

        sem_wait(&shmstruct_ptr->noverflow_mutex);
        noverflow_tmp = shmstruct_ptr->noverflow;
        sem_post(&shmstruct_ptr->noverflow_mutex);

        if (noverflow_last != noverflow_tmp) {
            printf("noverflow = %d\n", noverflow_tmp);
            noverflow_last = noverflow_tmp;
        }
    }

    return 0;
}
