#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/mman.h>

#define SEM_NAME "mysem"
#define FILENAME "myfile"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define NLOOP 100

int count = 0;

int main() {
    int zero = 0;
    int fd;
    int* ptr;
    int i;
    sem_t* mutex;

    fd = open(FILENAME, O_RDWR | O_CREAT, FILE_MODE);
    write(fd, &zero, sizeof (int));

    ptr = mmap(NULL, sizeof (int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    mutex = sem_open(SEM_NAME, O_CREAT | O_EXCL, FILE_MODE, 1);
    sem_unlink(SEM_NAME); /* удаляется из системы, но пока открыт */

    setbuf(stdout, NULL); /* stdout не буферизуется */

    if (fork() == 0) {
        for (i = 0; i < NLOOP; ++i) {
            sem_wait(mutex);
            printf("child: %d\n", (*ptr)++);
            sem_post(mutex);
        }
        exit(0);
    }

    for (i = 0; i < NLOOP; ++i) {
        sem_wait(mutex);
        printf("parent: %d\n", (*ptr)++);
        sem_post(mutex);
    }

    return 0;
}
