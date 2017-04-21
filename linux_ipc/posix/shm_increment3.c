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

struct shared {
    int count;
    sem_t mutex;
} shared = { 0, 0 };

int main() {
    int fd;
    int i;
    struct shared* ptr;

    fd = open(FILENAME, O_RDWR | O_CREAT, FILE_MODE);
    write(fd, &shared, sizeof(struct shared));

    ptr = mmap(NULL, sizeof(struct shared),
               PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    /* второй аргумент - ненулевой, чтобы семафор
     мог использоваться несколькими процессами*/
    sem_init(&ptr->mutex, 1, 1);

    setbuf(stdout, NULL); /* stdout не буферизуется */

    if (fork() == 0) {
        for (i = 0; i < NLOOP; ++i) {
            sem_wait(&ptr->mutex);
            printf("child: %d\n", ptr->count++);
            sem_post(&ptr->mutex);
        }
        exit(0);
    }

    for (i = 0; i < NLOOP; ++i) {
        sem_wait(&ptr->mutex);
        printf("parent: %d\n", ptr->count++);
        sem_post(&ptr->mutex);
    }

    sem_destroy(&ptr->mutex);

    return 0;
}
