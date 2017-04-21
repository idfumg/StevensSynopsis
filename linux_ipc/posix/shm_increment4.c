#include <stdio.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <string.h>

#define NLOOP 100

struct shared {
    int count;
    sem_t mutex;
} shared = { 0, 0 };

int main() {
    int fd;
    int i;
    struct shared* ptr;

    ptr = mmap(NULL, sizeof (struct shared),
               PROT_READ | PROT_WRITE,
               MAP_SHARED | MAP_ANON, -1, 0);

    memset(ptr, 0, sizeof (struct shared));

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
        return 0;
    }

    for (i = 0; i < NLOOP; ++i) {
        sem_wait(&ptr->mutex);
        printf("parent: %d\n", ptr->count++);
        sem_post(&ptr->mutex);
    }

    sem_destroy(&ptr->mutex);

    return 0;
}
