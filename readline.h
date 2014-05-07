#ifndef READLINE_H_
#define READLINE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/* Размер страницы памяти */
#define MEMORY_PAGE_SIZE sysconf(_SC_PAGESIZE)

/* Если верно условие state, выводим ошибку msg */
#define CHECK_RES(state, msg) do { \
    if (state) { \
        fprintf(stderr, msg); \
        exit(0); \
    } \
} while (0); \

/* Безопасное считывание строки */
void readline(char **line, FILE *pFile);

#endif /* READLINE_H_ */
