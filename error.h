#ifndef ERROR_H_
#define ERROR_H_

#include <stdio.h>
#include <stdlib.h>

/* Достигнут конец файлового потока */
#define END_OF_FILE -1

/* Успешное завершение */
#define SUCCESS_RETVAL 0

/* Не удалось выделить/перевыделить память */
#define NOT_ENOUGH_MEMORY 1

/* Несуществующий указатель */
#define INVALID_POINTER 2

/* Ошибка открытия файла */
#define ERROR_OPENING_FILE 3

/* Ошибка во время чтения данных из файлового потока */
#define FILE_ERROR 4

/* Номер ошибки */
extern int err_num;

/* Список сообщений об ошибке */
extern const char* err_messages[];

/* Напечатать сообщение об ошибке и завершиться */
void print_error(const char* msg);

#endif /*  ERROR_H_ */
