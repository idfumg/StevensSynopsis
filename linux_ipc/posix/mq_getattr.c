#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_RDWR)
#define MQ_NAME "/temp.1234"

int main() {
    mqd_t mqd;
    struct mq_attr attr;

    mqd = mq_open(MQ_NAME, FLAGS, FILE_MODE, NULL);

    if (mqd == (mqd_t) -1) {
        perror("message queue open failed");
        exit(1);
    }

    if (mq_getattr(mqd, &attr) == -1) {
        perror("get attributes of a message queue failed");
        exit(1);
    }

    printf("maxmsg: %ld, msgsize: %ld, curmsgs: %ld\n",
           attr.mq_maxmsg, attr.mq_msgsize, attr.mq_curmsgs);

    if (close(mqd) == -1) {
        perror("close message queue descriptor failed");
        exit(1);
    }

    return 0;
}
