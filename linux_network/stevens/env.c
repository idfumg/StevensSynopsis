#include <stdio.h>
#include <stdlib.h>

int main() {
    int backlog = -1;

    const char* ptr = getenv("LISTENQ");
    if (ptr != NULL)
        backlog = atoi(ptr);

    printf("backlog = %d\n", backlog);

    return 0;
}
