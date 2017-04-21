#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILENAME "myshm"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define FLAGS (O_RDWR | O_CREAT | O_EXCL)

int main() {
    int fd;
    off_t length = 100;
    char* ptr;

    fd = shm_open(FILENAME, FLAGS, FILE_MODE);
    ftruncate(fd, length);
    ptr = mmap(NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (!ptr) {
        perror("can't allocate shared memory");
        exit(1);
    }

    return 0;
}
