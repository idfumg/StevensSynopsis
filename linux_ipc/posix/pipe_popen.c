/*
  Читаем файл через открытие канала с помощью командной оболочки.
  Опция r - чтобы читать из открытого канала стандартный вывод утилиты.
  Опция w - чтобы писать в открытый канал на стандартный ввод утилиты.
*/

#include <stdio.h>

int main() {
	char buff[BUFSIZ] = "cat pipe_popen.c";
	FILE* stream = popen(buff, "r");

	while (fgets(buff, BUFSIZ, stream) != NULL)
		fputs(buff, stdout);

	pclose(stream);

	return 0;
}
