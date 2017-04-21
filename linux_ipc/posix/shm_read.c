#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FILENAME "myshm"
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    int fd = shm_open(FILENAME, O_RDONLY, FILE_MODE);

    struct stat stat;
    fstat(fd, &stat);
    char* ptr = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    close(fd);

    int c;
    int i;
    for (i = 0; i < stat.st_size; ++i){
        c = *ptr++;

        printf("ptr[%d] = %d\n", i, c);
        if (c != i % 256) {
            printf("ERROR! ptr[%d] = %d", i, c);
            exit(1);
        }
    }

    return 0;
}
