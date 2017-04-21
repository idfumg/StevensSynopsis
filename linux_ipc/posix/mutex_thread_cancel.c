/*
  Пример отмены потока другим потоком с помощью
  функции pthread_cancel.
  Также используется механизм очистки, который вызывается
  автоматически, если данный поток был отменен вызовом
  pthread_cancel.
  Функция очистки просто выводит информационное сообщение.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

void thread_cancel_callback(void* arg) {
    printf("forever_fn: Somebody killed me!"
           "Cleanup and unlock everything!\n");
}

void* forever_fn(void* arg) {
    pthread_cleanup_push(thread_cancel_callback, NULL);
    for (;;) {
        pthread_rwlock_rdlock(&shared.mutex);
        printf("forever_fn(): i'll work forever!!!\n");
        sleep(1);
        pthread_rwlock_unlock(&shared.mutex);
    }
    pthread_cleanup_pop(1);
}

int main() {
    pthread_t forever_thread;

    pthread_create(&forever_thread, NULL, forever_fn, NULL);

    printf("main(): now i kill forever thread!\n");
    pthread_cancel(forever_thread);

    void* res;
    pthread_join(forever_thread, &res);

    if (res == PTHREAD_CANCELED)
        printf("main(): forever thread was cancelled!\n");

    return 0;
}
