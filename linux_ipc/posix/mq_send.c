#include <stdlib.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MQ_NAME "/temp.1234"
#define LEN 100
#define PRIO 2

int main() {
    mqd_t mqd;
    void *ptr;

    mqd = mq_open(MQ_NAME, O_WRONLY);

    if (mqd == -1) {
        perror("open message queue failed");
        exit(1);
    }

    ptr = calloc(LEN, sizeof(char));

    if (ptr == NULL) {
        perror("allocate memory failed");
        exit(1);
    }

    mq_send(mqd, ptr, LEN, PRIO);

    free(ptr);

    return 0;
}
