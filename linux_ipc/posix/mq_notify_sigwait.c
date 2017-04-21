#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>

#define MQ_NAME "/temp.1234"

/*
Функция sigwait часто используется в многопоточных программах.
Однако, в многопоточных программах нельзя использовать sigprocmask -
вместо нее следует использовать pthread_procmask.
Существует, также, функция sigtimedwait, которая возвращает структуру
siginfo_t и позволяет вызывающему процеесу установить ограничение
по времени на ожидание.
Рекомендуется использовать sigwait для обработки сигналов в
многопоточном процессе и не использовать асинхронные обработчики.
 */
int main() {
    int rc;
    int n;
    int signo;
    mqd_t mqd;
    void* msg;
    sigset_t newmask;
    struct mq_attr attr;
    struct sigevent sigev;

    mqd = mq_open(MQ_NAME, O_RDONLY);

    if (mqd == -1) {
        perror("open messge queue failed");
        exit(1);
    }

    rc = mq_getattr(mqd, &attr);

    if (rc == -1) {
        perror("get mqueue attibutes failed");
        exit(1);
    }

    msg = calloc(attr.mq_msgsize, sizeof(char));

    if (msg == NULL) {
        perror("allocate memory for msg failed");
        exit(1);
    }

    rc = sigemptyset(&newmask);

    if (rc == -1) {
        perror("empt signal set failed");
        exit(1);
    }

    rc = sigaddset(&newmask, SIGUSR1);

    if (rc == -1) {
        perror("add  SIGUSR1 in newmask failed");
        exit(1);
    }

    rc = sigprocmask(SIG_BLOCK, &newmask, NULL);

    if (rc == -1) {
        perror("set block for new sig mask failed");
        exit(1);
    }

    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;

    rc = mq_notify(mqd, &sigev);

    if (rc == -1) {
        perror("set notify for mqueue failed");
        exit(1);
    }

    while (1) {
        sigwait(&newmask, &signo);

        if (signo == SIGUSR1) {
            rc = mq_notify(mqd, &sigev);

            if (rc == -1) {
                perror("set notify for mqueue failed");
                exit(1);
            }

            while ((n = mq_receive(mqd, msg, attr.mq_msgsize,
                                   NULL)) >= 0) {
                printf("Received %ld bytes\n", (long) n);
            }

            if (errno != EAGAIN) {
                perror("mq_receive failed");
                exit(1);
            }
        }
    }

    free(msg);

    return 0;
}
