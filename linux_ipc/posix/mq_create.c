#include <stdlib.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_RDWR | O_CREAT | O_EXCL)
#define MQ_NAME "/temp.1234"
#define MQ_MAXMSG 10
#define MQ_MSGSIZE 8192

/* Максимальное количество сообщений в очереди устанавливается жестко ядром.
 Это значение можно поменять в /proc/sys/fs/mqueue/msg_max.
 Максимальный размер сообщения можно поменять там же.*/
int main() {
    mqd_t mqd;
    struct mq_attr attr;

    attr.mq_maxmsg = (long) MQ_MAXMSG;
    attr.mq_msgsize = (long) MQ_MSGSIZE;

    mqd = mq_open(MQ_NAME, FLAGS, FILE_MODE, &attr);

    if (mqd == -1) {
        perror("open message queue failed");
        exit(1);
    }

    close(mqd);

    return 0;
}
