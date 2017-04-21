#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include <errno.h>

#define NLOOP 20

#define MESGSIZE 256
#define NMESG 16

#define SHMNAME "myshm"
#define SEMNAME "mysem"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

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
    int fd = shm_open(SHMNAME, O_RDWR, FILE_MODE);
    struct shmstruct* shmstruct_ptr = mmap(NULL, sizeof (struct shmstruct),
                                           PROT_READ | PROT_WRITE,
                                           MAP_SHARED, fd, 0);
    close(fd);

    shmstruct_ptr->noverflow = 0;
    pid_t pid = getpid();

    int i;
    int offset;
    char msg[MESGSIZE];
    for (i = 0; i < NLOOP; ++i) {
        usleep(100);
        snprintf(msg, MESGSIZE, "[%ld]: message %d", (long) pid, i);

        if (sem_trywait(&shmstruct_ptr->nempty) == -1) {
            if (errno == EAGAIN) {
                sem_wait(&shmstruct_ptr->noverflow_mutex);
                shmstruct_ptr->noverflow++;
                sem_post(&shmstruct_ptr->noverflow_mutex);
                continue;
            }

            perror("sem_trywait error");
            exit(1);
        }

        sem_wait(&shmstruct_ptr->mutex);
        offset = shmstruct_ptr->msgoff[shmstruct_ptr->nput];
        if (++(shmstruct_ptr->nput) >= NMESG)
            shmstruct_ptr->nput = 0;
        sem_post(&shmstruct_ptr->mutex);
        strcpy(&shmstruct_ptr->msgdata[offset], msg);
        sem_post(&shmstruct_ptr->nstored);
    }

    return 0;
}
