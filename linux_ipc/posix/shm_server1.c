#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>

#define SHMNAME "myshm"
#define SEMNAME "mysem"

#define FILE_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define SEM_FLAGS (O_CREAT | O_EXCL)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

struct shmstruct {
    int count;
};

sem_t* mutex;

int main() {
    int fd;
    struct shmstruct* shmstruct_ptr;

    shm_unlink(SHMNAME);
    fd = shm_open(SHMNAME, FILE_FLAGS, FILE_MODE);
    ftruncate(fd, sizeof (struct shmstruct));
    shmstruct_ptr = mmap(NULL, sizeof (struct shmstruct),
                         PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    sem_unlink(SEMNAME);
    mutex = sem_open(SEMNAME, SEM_FLAGS, FILE_MODE, 1);
    sem_close(mutex);

    return 0;
}
