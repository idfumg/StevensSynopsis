#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>

/*
1. Если аргумент notification ненулевой, процесс регистрируется на
уведомление для данной очереди.
2. Если аргумент notification нулевой, уведомление для процесса
отключается.
3. Только один процесс может быть зарегистрирован на
уведомление для любой данной очереди в любой момент.
4. Блокировка в вызове mq_receive имеет приоритет перед любой
регистрацией на уведомление.
5. При отправке уведомление зарегистрированному процессу,
регистрация снимается и процесс должен зарегистрироваться снова,
вызвав mq_notify еще раз.
 */

#define MQ_NAME "/temp.1234"

volatile sig_atomic_t mqflag;

static void sigusr1(int signo) {
    mqflag = 1;
}

int main() {
    struct mq_attr attr;
    struct sigevent sigev;
    void* buff;
    mqd_t mqd;
    int rc;
    int n;
    sigset_t zeromask, newmask, oldmask;

    mqd = mq_open(MQ_NAME, O_RDONLY | O_NONBLOCK);

    if (mqd == -1) {
        perror("open message queue failed");
        exit(1);
    }

    rc = mq_getattr(mqd, &attr);

    if (rc == -1) {
        perror("get message queue attributes failed");
        exit(1);
    }

    buff = calloc(attr.mq_msgsize, sizeof(char));

    if (buff == NULL) {
        perror("allocate memory for message failed");
        exit(1);
    }

    rc = sigemptyset(&zeromask);

    if (rc == -1) {
        perror("set signal zero mask to zero failed");
        exit(1);
    }

    rc = sigemptyset(&newmask);

    if (rc == -1) {
        perror("set signal new mask to zero failed");
        exit(1);
    }

    rc = sigemptyset(&oldmask);

    if (rc == -1) {
        perror("set signal old mask to zero failed");
        exit(1);
    }

    rc = sigaddset(&newmask, SIGUSR1);

    if (rc == -1) {
        perror("add to new signal mask SIGUSR1 signo failed");
        exit(1);
    }

    signal(SIGUSR1, sigusr1);
    sigev.sigev_notify = SIGEV_SIGNAL;
    sigev.sigev_signo = SIGUSR1;

    rc = mq_notify(mqd, &sigev);

    if (rc == -1) {
        perror("set message queue notify failed");
        exit(1);
    }

    while (1) {
        rc = sigprocmask(SIG_BLOCK, &newmask, &oldmask);

        if (rc == -1) {
            perror("bloking SIGUSR1 signal failed");
            exit(1);
        }

        while (mqflag == 0)
            sigsuspend(&zeromask);
        mqflag = 0;

        rc = mq_notify(mqd, &sigev);

        if (rc == -1) {
            perror("set message queue notify failed");
            exit(1);
        }

        while ((n = mq_receive(mqd, buff, attr.mq_msgsize,
                               NULL)) >= 0)
            printf("Read %ld bytes\n", (long)n);

        if (errno != EAGAIN) {
            perror("receive msg from message queue failed");
            exit(1);
        }

        rc = sigprocmask(SIG_UNBLOCK, &newmask, NULL);

        if (rc == -1) {
            perror("unbloking SIGUSR1 signal failed");
            exit(1);
        }
    }

    return 0;
}
