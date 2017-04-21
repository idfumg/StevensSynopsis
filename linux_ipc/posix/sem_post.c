#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define SEM_FILE "/mysem"

int main() {
    sem_t* sem;
    int value;

    sem = sem_open(SEM_FILE, 0);
    sem_post(sem);
    sem_getvalue(sem, &value);
    printf("value = %d\n", value);

    return 0;
}
