/*
  Использование семафоров. Неименованные семафоры.
  Задача производителя-потребителя.
  Реализация нескольких производителей и нескольких
  потребителей.
  Применен ограниченный по размеру кольцевой буфер.
  (shared.nput % BUFF позволяет вставлять по кругу с
  учетом того, что контролируется элемент, который
  должен быть помещен в буфер, и элемент, которы
  из буфера должен быть взят. Возможно, не стоит
  использовать такую реализацю, поскольку
  возможно переполнение nput.).
  Поскольку, здесь используется обычное взаимное
  исключение, доступ к данным может иметь только один поток.
  Возможно использование блокировок на чтение-запись,
  если возможно иметь несколько читателей, не изменяющий
  данных. Семафоры же, обеспечат роль условных переменных,
  ожидая заполнения/опустошения буфера.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define NITEMS 1000000
#define NBUFF 100
#define MAX_PRODUCERS 3
#define MAX_CONSUMERS 1

struct {
    int buff[NBUFF];
    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
    int nput;
    int nputval;
} shared = { {0}, 0, 0, 0, 0, 0 };

void* produce(void* arg) {
    for (;;) {
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);

        if (shared.nput >= NITEMS) {
            sem_post(&shared.nempty);
            sem_post(&shared.mutex);
            return NULL;
        }

        shared.buff[shared.nput % NBUFF] = shared.nputval;
        shared.nput++;
        shared.nputval++;

        sem_post(&shared.mutex);
        sem_post(&shared.nstored);
    }
}

void* consume(void* arg) {
    int i;

    for (i = 0; i < NITEMS; ++i) {
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);
        //printf("buff[%d] = %d\n", i, shared.buff[i % NBUFF]);
        sem_post(&shared.mutex);
        sem_post(&shared.nempty);
    }
}

int main() {
    int i;
    pthread_t producers[MAX_PRODUCERS];
    pthread_t consumers[MAX_CONSUMERS];

    sem_init(&shared.mutex, 0, 1);
    sem_init(&shared.nempty, 0, NBUFF);
    sem_init(&shared.nstored, 0, 0);

    for (i = 0; i < MAX_PRODUCERS; ++i)
        pthread_create(&producers[i], NULL, produce, NULL);

    for (i = 0; i < MAX_CONSUMERS; ++i)
        pthread_create(&consumers[i], NULL, consume, NULL);

    for (i = 0; i < MAX_PRODUCERS; ++i)
        pthread_join(producers[i], NULL);

    for (i = 0; i < MAX_CONSUMERS; ++i)
        pthread_join(consumers[i], NULL);

    sem_destroy(&shared.mutex);
    sem_destroy(&shared.nempty);
    sem_destroy(&shared.nstored);

    return 0;
}
