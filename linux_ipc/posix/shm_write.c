#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FILENAME "myshm"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    int fd = shm_open(FILENAME, O_RDWR, FILE_MODE);

    struct stat stat;
    fstat(fd, &stat);

    char* ptr = mmap(NULL, stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    int i;
    for (i = 0; i < stat.st_size; ++i)
        *ptr++ = i % 256;

    return 0;
}
