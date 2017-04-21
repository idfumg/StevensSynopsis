#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define SEM_FILE "/mysem"

int main() {
    sem_t* sem;
    int value;

    sem = sem_open(SEM_FILE, 0);

    if (sem == SEM_FAILED) {
        perror("semaphore open failed!");
        exit(1);
    }

    sem_getvalue(sem, &value);
    printf("semaphore count value = %d\n", value);

    sem_close(sem);

    return 0;
}
