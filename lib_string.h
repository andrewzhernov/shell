#ifndef LIB_STRING_H_
#define LIB_STRING_H_

#include <stdio.h>
#include <stdlib.h>

#include "error.h"

/* Количество дополнительно выделяемых байт в случае недостатка памяти */
#define REALLOCATION_SIZE 8

struct string {
    char* line;
    int size;
    int capacity;
};

typedef struct string string_t;

/* Создать строку */
int string_create(string_t** str);

/* Удалить строку */
void string_destroy(string_t* str);

/* Добавить символ в конец строки */
int string_push_back(string_t* str, char symbol);

#endif /* LIB_STRING_H_ */
