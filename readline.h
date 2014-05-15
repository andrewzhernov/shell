#ifndef READLINE_H_
#define READLINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

#include "error.h"

/* Размер страницы памяти */
#define MEMORY_PAGE_SIZE sysconf(_SC_PAGESIZE)

/* Безопасное считывание строки */
int readline(FILE* pFile, char** line, char delim);

/* Вывод сообщения об ошибке и завершение */
void readline_perror(const char* msg);

#endif /* READLINE_H_ */
