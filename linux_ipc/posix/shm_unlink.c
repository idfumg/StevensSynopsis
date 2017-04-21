#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILENAME "myshm"

int main() {
    int rc = shm_unlink(FILENAME);
    if (rc == -1) {
        perror("can't close shared memory!");
        exit(1);
    }

    return 0;
}
