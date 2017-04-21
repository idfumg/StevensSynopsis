/*
  Задача производителя-потребителя.
  Мы не знаем, когда необходимо считывать значения из массива.
  Поэтому, нам остается проверять значение количества записанных
  элементов, устанавливая и снимая блокировку.
  Это называется опросом - polling и является лишней тратой
  процессорного времени.
  В данном случае, необходимо использовать какое-то средство
  синхронизации, позволяющее потоку приостанавливать работу,
  пока не произойдет какое-либо событие.
  Взаимное исключение (mutable exclusion) для ожидания не
  используется.
  Для ожидания используется условная переменная.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_ITEMS 1000000
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

int consume_wait() {
    int wait;

    pthread_mutex_lock(&shared.mutex);
    wait = shared.nitems < MAX_ITEMS;
    pthread_mutex_unlock(&shared.mutex);

    return wait == 1;
}

void* consume(void* arg) {
    int i;

    for (;;)
        if (!consume_wait())
            break;

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
