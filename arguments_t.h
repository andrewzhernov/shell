#ifndef CONTAINERS_H_
#define CONTAINERS_H_

#include <stdlib.h>

#include "lib_string.h"
#include "error.h"

/* Количество дополнительно выделяемых байт в случае недостатка памяти */
#define REALLOCATION_SIZE 8

/* Произошла ошибка */
#define CONTAINERS_ERROR -1

/* Успешное завершение функции */
#define CONTAINERS_SUCCESS_RETVAL 0

/* Не удалось выделить/перевыделить память */
#define CONTAINERS_NOT_ENOUGH_MEMORY 1

struct arguments {
    char** options;
    int size;
    int capacity;
    int opt_size;
    int opt_capacity;
};

typedef struct arguments arguments_t;

/* Создать */
int arguments_create(arguments_t** args);

/* Удалить */
void arguments_destroy(arguments_t* args);

/* Добавить элемент в конец */
int arguments_push_back(arguments_t* args, char symbol);

#endif /* CONTAINERS_H_ */
