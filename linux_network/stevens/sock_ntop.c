#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>

/*
  inet_ntop, не зависящий от протокола.
  нужно написать остальное, по примерам из книги (скачать).
*/

char* sock_ntop(const struct sockaddr* sa, socklen_t sa_len) {
    char port_str[7];
    static char str[128];

    switch (sa->sa_family) {
    case AF_INET: {
        struct sockaddr_in* sin = (struct sockaddr_in*) sa;

        if (inet_ntop(AF_INET, &sin->sin_addr, str, sizeof(str)) == NULL)
            return NULL;

        if (ntohs(sin->sin_port) != 0) {
            snprintf(port_str, sizeof(port_str), ".%d", ntohs(sin->sin_port));
            strcat(str, port_str);
        }

        return str;
    }
    }

    errno =  EAFNOSUPPORT;
    return NULL;
}
