/*
  Задача производителя-потребителя. Общий случай.
  Запускаются несколько потоков производителей и
  несколько потоков потребителей.
  В данной задаче используются взаимные исключение
  на чтение и на запись.
  Несколько потоков, ожидающие блокировки на чтение,
  могут получить ее одновременно, если ни один поток
  не получил блокировки на запись.
  Только один поток может получить блокировку на запись.
  Потоки просто заполняют элементы массива.
  Блокировки были введены искусственно, чтобы протестировать
  одновременный доступ к критической области.
  Потребитель на самом деле не потребляет, а просто
  просматривает все записи в массиве и, если одна из
  них не заполнена, то продолжает свою работу.
  (ждет оставшиеся данные). Как только весь массив заполнен,
  потребители заканчивают свою работу. (считается, что все
  данные были обработаны).
  Также здесь приводится пример использования функции
  временного ожидания блокировки pthread_rwlock_tryrdlock.
  Данная функция каждую секунду просматривает массив на
  заполненность. Если массив еще не заполнен, то функция
  выводит информационное сообщение. Если массив
  заполнен, вывод содержимое массива.
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_WRITE_THREADS 6
#define MAX_READ_THREADS 6
#define MAX_ITEMS 1000000

struct {
    pthread_rwlock_t mutex;
    int buff[MAX_WRITE_THREADS];
} shared = {
    PTHREAD_RWLOCK_INITIALIZER,
    {0}
};

void* produce(void* arg) {
    const long thread_num = (long) arg;

    printf("[%ld] produce thread started.\n", thread_num);

    for (;;) {
        pthread_rwlock_wrlock(&shared.mutex);

        if (shared.buff[thread_num] >= MAX_ITEMS) {
            printf("[%ld] produce thread done. exit.\n", thread_num);
            pthread_rwlock_unlock(&shared.mutex);
            return NULL;
        }
        shared.buff[thread_num]++;

        pthread_rwlock_unlock(&shared.mutex);
    }
}

void* consume(void* arg) {
    const long thread_num = (long) arg;
    int need_continue;
    int i;

    printf("[%ld] consume thread started.\n", thread_num);

    for (;;) {
        need_continue = 0;

        pthread_rwlock_rdlock(&shared.mutex);

        for (i = 0; i < MAX_WRITE_THREADS; ++i)
            if (shared.buff[i] < MAX_ITEMS) {
                need_continue = 1;
                break;
            }

        pthread_rwlock_unlock(&shared.mutex);

        if (need_continue == 0) {
            printf("[%ld] consume thread done. exit.\n", thread_num);
            return NULL;
        }

        usleep(1);
    }
}

void* total_printer(void* arg) {
    int work = 1;
    int i;

    while (work) {
        if (pthread_rwlock_tryrdlock(&shared.mutex) != 0) {
            printf("total_printer still wait\n");
            sleep(1);
            continue;
        }

        for (i = 0; i < MAX_WRITE_THREADS; ++i)
            if (shared.buff[i] == MAX_ITEMS) {
                work = 0;
                break;
            }

        pthread_rwlock_unlock(&shared.mutex);
    }

    for (i = 0; i < MAX_WRITE_THREADS; ++i)
        printf("buff[%ld] = %d\n", i, shared.buff[i]);
}

int main() {
    long i;
    pthread_t produce_threads[MAX_WRITE_THREADS];
    pthread_t consume_threads[MAX_READ_THREADS];
    pthread_t total_printer_thread;

    for (i = 0; i < MAX_WRITE_THREADS; ++i)
        pthread_create(&produce_threads[i], NULL,
                       produce, (void*) i);

    for (i = 0; i < MAX_READ_THREADS; ++i)
        pthread_create(&consume_threads[i], NULL,
                       consume, (void*) i);

    pthread_create(&total_printer_thread, NULL,
                   total_printer, NULL);

    for (i = 0; i < MAX_WRITE_THREADS; ++i)
        pthread_join(produce_threads[i], NULL);

    for (i = 0; i < MAX_READ_THREADS; ++i)
        pthread_join(consume_threads[i], NULL);

    pthread_join(total_printer_thread, NULL);

    return 0;
}
