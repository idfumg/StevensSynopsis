#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SEM_FILE "/mysem"
#define SEM_FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    unsigned int value;
    sem_t* sem;
    sem = sem_open(SEM_FILE, SEM_FLAGS, FILE_MODE, value);

    if (sem == SEM_FAILED) {
        perror("create semaphore failed!");
        exit(1);
    }

    sem_close(sem);

    return 0;
}
