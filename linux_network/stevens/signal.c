#include <stdio.h>
#include <error.h>
#include <signal.h>

typedef void (*sigfunc)(int signo);

sigfunc Signal(int signo, sigfunc fn) {
    struct sigaction act, oact/* Old signal action. */;

    act.sa_handler = fn;
    sigemptyset(&act.sa_mask);
    if (signo != SIGALRM) { /* We want interrupt system call when io. */
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }
    if (sigaction(signo, &act, &oact) < 0) {
        perror("sigaction error");
        return SIG_ERR;
    }

    return oact.sa_handler;
}
