#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MQ_NAME "/temp.1234"
#define FLAGS (O_RDONLY | O_NONBLOCK)

/* Перед получением сообщения из очереди, необходимо
 обязательно считать ее атрибуты и выделить память
 для буфера, в котором будет сохранено сообщение.
 O_NONBLOCK позволяет сделать чтение неблокирующим.*/
int main() {
    mqd_t mqd;
    struct mq_attr attr;
    void* buff;
    unsigned int prio;
    long n;
    int rc;

    mqd = mq_open(MQ_NAME, FLAGS);

    if (mqd == -1) {
        perror("open message queue failed");
        exit(1);
    }

    rc = mq_getattr(mqd, &attr);

    if (rc == -1) {
        perror("get mq attributes failed");
        exit(1);
    }

    buff = calloc(attr.mq_msgsize, sizeof(char));

    if (buff == NULL) {
        perror("alloate memory failed");
        exit(1);
    }

    n = mq_receive(mqd, buff, attr.mq_msgsize, &prio);

    if (n == -1) {
        perror("receive message from mq failed");
        exit(1);
    }

    printf("read %ld bytes, priority = %u\n", (long) n, prio);

    free(buff);

    return 0;
}
