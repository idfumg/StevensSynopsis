#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define NLOOP 50

#define SHMNAME "myshm"
#define SEMNAME "mysem"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shmstruct {
    int count;
};

sem_t* mutex;

int main() {
    int fd;
    int i;
    pid_t pid;
    struct shmstruct* shmstruct_ptr;

    fd = shm_open(SHMNAME, O_RDWR, FILE_MODE);
    shmstruct_ptr = mmap(NULL, sizeof (struct shmstruct),
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    mutex = sem_open(SEMNAME, 0);

    pid = getpid();

    for (i = 0; i < NLOOP; ++i) {
        sem_wait(mutex);
        printf("[%d]: count = %d\n", (long) pid, shmstruct_ptr->count++);
        sem_post(mutex);
    }

    return 0;
}
