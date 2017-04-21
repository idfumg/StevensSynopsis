#include <fcntl.h>

int fcntl_setflag(int fd, int flag) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
        return -1;

    flags |= flag;

    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

    return 0;
}

int fcntl_rmflag(int fd, int flag) {
    int flags;

    if ((flags = fcntl(fd, F_GETFL, 0)) < 0)
        return -1;

    flags &= ~flag;

    if (fcntl(fd, F_SETFL, flags) < 0)
        return -1;

    return 0;
}
