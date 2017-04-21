#include <stdio.h>
#include <unistd.h>

int main() {
    printf("MQ_OPEN_MAX = %ld, MQ_PRIO_MAX = %ld\n",
           sysconf(_SC_MQ_OPEN_MAX), sysconf(_SC_MQ_PRIO_MAX));

    return 0;
}
