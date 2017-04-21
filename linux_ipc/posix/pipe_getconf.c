/*
  Различные установки в системе можно получить,
  используя утилиту getconf в интерпретаторе.
*/

#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("usage: %s <pathname>\n", argv[0]);
        return 1;
    }

    printf("PIPE_BUF = %ld, OPEN_MAX = %ld\n",
           pathconf(argv[1], _PC_PIPE_BUF),
           sysconf(_SC_OPEN_MAX));

    return 0;
}
