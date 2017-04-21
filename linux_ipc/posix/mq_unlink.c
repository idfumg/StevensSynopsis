#include <stdlib.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>

#define MQ_NAME "/temp.1234"

int main() {
    if (mq_unlink(MQ_NAME) == -1) {
        perror("unlink message queue failed");
        exit(1);
    }

    return 0;
}
