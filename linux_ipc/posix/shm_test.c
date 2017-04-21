#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define FILENAME "myshm"
#define FILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

int main() {
    shm_unlink(FILENAME);

    int fd1 = shm_open(FILENAME, O_RDWR | O_CREAT | O_EXCL, FILEMODE);
    ftruncate(fd1, sizeof(int));

    int fd2 = open("/etc/motd", O_RDONLY);
    struct stat stat;
    fstat(fd2, &stat);

    pid_t childpid;
    int* ptr1;
    int* ptr2;

    if ((childpid = fork()) == 0) {
        ptr2 = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd2, 0);
        ptr1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);

        printf("child: shm ptr = %p, motd ptr = %p\n", ptr1, ptr2);
        sleep(5);
        printf("shared memory integer = %d\n", *ptr1);
        exit(0);
    }

    ptr1 = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd1, 0);
    ptr2 = mmap(NULL, stat.st_size, PROT_READ, MAP_SHARED, fd2, 0);

    printf("parent: shm ptr = %p, motd ptr = %p\n", ptr1, ptr2);
    *ptr1 = 777;

    waitpid(childpid, NULL, 0);

    return 0;
}
