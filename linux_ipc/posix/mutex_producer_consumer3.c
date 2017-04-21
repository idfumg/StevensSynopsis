/*
  Задача производителя-потребителя.
  В данном варианте используется условная переменная для
  ожидания.
  Также отличие от предыдущих вариантов в том, что
  запуск потока-потребителя происходит сразу после запуска
  потоков производителей.
  После пробуждения потока, необходимо все равно проверять
  необходимое условие, поскольку возможно ложное пробуждение.
  Оверхед небольшой - две проверки условия.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_ITEMS 10000000
#define MAX_THREADS 10

struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int buff[MAX_THREADS];
    int nitems;
} shared = {
    PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_COND_INITIALIZER
};

void* produce(void* arg) {
    for (;;) {
        pthread_mutex_lock(&shared.mutex);

        if (shared.nitems >= MAX_ITEMS) {
            pthread_cond_signal(&shared.cond);
            pthread_mutex_unlock(&shared.mutex);
            return NULL;
        }

        shared.buff[*((int*) arg)]++;
        shared.nitems++;

        pthread_mutex_unlock(&shared.mutex);
    }
}

void* consume(void* arg) {
    pthread_mutex_lock(&shared.mutex);

    while (shared.nitems < MAX_ITEMS)
        pthread_cond_wait(&shared.cond, &shared.mutex);

    pthread_mutex_unlock(&shared.mutex);

    int i;
    for (i = 0; i < MAX_THREADS; ++i)
        printf("buff[%d] = %d\n", i, shared.buff[i]);

    return NULL;
}

int main() {
    int i;
    pthread_t produce_threads[MAX_THREADS];
    pthread_t consume_thread;

    for (i = 0; i < MAX_THREADS; ++i)
        pthread_create(&produce_threads[i], NULL,
                       produce, &i);
    pthread_create(&consume_thread, NULL, consume, NULL);

    for (i = 0; i < MAX_THREADS; ++i)
        pthread_join(produce_threads[i], NULL);
    pthread_join(consume_thread, NULL);

    return 0;
}
