#include <stdio.h>
#include <errno.h>

ssize_t readn(int fd, void* vbuf, ssize_t n) {
    ssize_t nleft;
    ssize_t nread;
    char* ptr;

    ptr = vbuf;
    nleft = n;

    while (nleft > 0) {
        if ((nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (nread == 0) /* EOF */
            break;

        nleft -= nread;
        ptr += nread;
    }

    return n - nleft;
}
