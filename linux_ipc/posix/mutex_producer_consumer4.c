/*
  Задача производителя-потребителя.
  В данном варианте используется функция broadcast для
  условной переменной. Она пробуждает все потоки, ожидающие
  на данной условной переменной.
  Потоки засыпают с помощью timedwait на 2 секунды для того,
  чтобы была возможность выполнять какие-то действия, если
  в течение некоторого времени не произошло пробуждение
  по условной переменной.
  В данном случае, просто выводится информационное сообщение.
  После пробуждения потока, необходимо все равно проверять
  необходимое условие, поскольку возможно ложное пробуждение.
  Оверхед небольшой - две проверки условия.
  Можно использовать условные переменные в случае, если
  имеется несколько читателей и один или несколько потребителей.
  В этом случае можно оповестить сразу всех читателей,
  чтобы они могли обработать новые данные.
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_ITEMS 50000000
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
            pthread_cond_broadcast(&shared.cond);
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

    while (shared.nitems < MAX_ITEMS) {
        const struct timespec abstime = {
            time(0) + 2,
            0
        };
        pthread_cond_timedwait(&shared.cond, &shared.mutex,
                               &abstime);
        printf("timed out try one else\n");
    }

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
