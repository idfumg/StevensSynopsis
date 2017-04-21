/*
  Задача производителя-потребителя.
  Критическая область данных представляет собой структуру,
  которая защищается взаимным исключением.
  Запускаются несколько производителей и один потребитель.
  Каждый производитель увеличивает свой элемент в массиве,
  пока количество элементов не будет равно максимальному.
  Взаимное исключение было добавлено искуственно.
  На самом деле оно не нужно.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_ITEMS 100000000
#define MAX_THREADS 10

struct {
    pthread_mutex_t mutex;
    int buff[MAX_THREADS];
    int nitems;
} shared = {
    PTHREAD_MUTEX_INITIALIZER
};

void* produce(void* arg) {
    for (;;) {
        pthread_mutex_lock(&shared.mutex);

        if (shared.nitems >= MAX_ITEMS) {
            pthread_mutex_unlock(&shared.mutex);
            return NULL;
        }

        shared.buff[*((int*) arg)]++;
        shared.nitems++;

        pthread_mutex_unlock(&shared.mutex);
    }
}

void* consume(void* arg) {
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
        pthread_create(&produce_threads[i], NULL, produce, &i);

    for (i = 0; i < MAX_THREADS; ++i)
        pthread_join(produce_threads[i], NULL);

    pthread_create(&consume_thread, NULL, consume, NULL);
    pthread_join(consume_thread, NULL);

    return 0;
}
