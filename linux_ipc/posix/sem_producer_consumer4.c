/*
  Использование семафоров.
  Задача производителя-потребителя.
  В данной реализации добавлены новые переменные
  в структуру, подлежащую синхронизации.
  Читатель заменен таким образом, чтобы он не знал
  о том, сколько элементов уже прочитано.
  Вся эта информация хранится в структуре.
  Необходимо дополнительно увеличение семафора stored
  в производителе, иначе программа зависнет.
  Это произойдет потому, что некоторые из читателей
  не знают, когда нужно остановиться. Они ждут увелечение
  счетчика shared.nstored, но он никогда не увеличится,
  если производители закончили свою работу.
  Поэтому каждый производитель должен дать шанс
  потребителям завершить свою работу.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>

#define NITEMS 1000000
#define NBUFF 1000000
#define MAX_PRODUCERS 3
#define MAX_CONSUMERS 2

struct {
    int buff[NBUFF];
    sem_t mutex;
    sem_t nempty;
    sem_t nstored;
    int nput;
    int nputval;
    int nget;
    int ngetval;
} shared = { {0}, 0, 0, 0, 0, 0, 0, 0 };

void* produce(void* arg) {
    for (;;) {
        sem_wait(&shared.nempty);
        sem_wait(&shared.mutex);

        if (shared.nput >= NITEMS) {
            sem_post(&shared.nstored);
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
    for (;;) {
        sem_wait(&shared.nstored);
        sem_wait(&shared.mutex);

        if (shared.nget >= NITEMS) {
            sem_post(&shared.nstored);
            sem_post(&shared.mutex);
            return NULL;
        }

        //int i;
        //i = shared.nget % NBUFF;
        //printf("buff[%d] = %d\n", i, shared.buff[i]);

        shared.nget++;
        shared.ngetval++;

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
