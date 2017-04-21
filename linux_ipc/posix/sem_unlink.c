#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define SEM_FILE "/mysem"

int main() {
    int rc = sem_unlink(SEM_FILE);

    if (rc == -1) {
        perror("semaphore unlink failed!");
        exit(1);
    }

    return 0;
}
