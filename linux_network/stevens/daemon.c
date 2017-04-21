/*
  Задает процессу свойства демона.
  Первый fork заставлет закрыться процесс, порожденный терминалом.
  Первый дочерний процесс устанавливается как владелец сессии.
  После второго вызова fork нет шансов, что процесс может
  заблокировать терминал снова.
  Игнорируется сигнал SIGHUP, потому что он отправляется всем
  процессам порожденным от главного процесса сессии, по его
  завершении.
  Исключается возможность использования стандартного ввода и
  вывода закрытием дескрипторов и, затем, связывания их со
  устройсвом null.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <error.h>

#define MAXFD 64

int to_daemon() {
    int i;
    pid_t pid;

    if ((pid = fork()) < 0)
        return -1;
    else if (pid)
        exit(0);

    if (setsid() < 0)
        return -1;

    Signal(SIGHUP, SIG_IGN);

    if ((pid = fork()) < 0)
        return -1;
    else if (pid)
        exit(0);

    chdir("/");

    for (i = 0; i < MAXFD; ++i)
        close(i);

    open("/dev/null", O_RDONLY);
    open("/dev/null", O_RDWR);
    open("/dev/null", O_RDWR);

    return 0;
}
