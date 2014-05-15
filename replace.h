#ifndef REPLACE_H_
#define REPLACE_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* Размер страницы памяти */
#define REPLACE_PAGE_SIZE sysconf(_SC_PAGESIZE)

/* Успешно прочитана строка */
#define REPLACE_SUCCESS_RETVAL 0

/* Не удалось выделить память */
#define REPLACE_NOT_ENOUGH_MEMORY 1

/* Несуществующий указатель на строку */
#define REPLACE_INVALID_POINTER 2

/* Пустой шаблон заменяемой подстроки */
#define REPLACE_EMPTY_PATTERN 3

/* Обработчик функции replace() */
#define REPLACE_HANDLER(id) { \
    switch(id) { \
        case REPLACE_SUCCESS_RETVAL: \
            break; \
        case REPLACE_NOT_ENOUGH_MEMORY: \
            fprintf(stderr, "failed to allocate memory\n"); \
            exit(id); \
        case REPLACE_INVALID_POINTER: \
            fprintf(stderr, "invalid pointer\n"); \
            exit(id); \
        case REPLACE_EMPTY_PATTERN: \
            fprintf(stderr, "pattern is empty\n"); \
            exit(id); \
    } \
}

/* Замена одной строки на другую */
int replace(char** text, char* pattern, char* target);

#endif
