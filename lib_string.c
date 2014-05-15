#include "lib_string.h"

int string_create(string_t** str) {
    if (str == NULL) {
        return err_num = INVALID_POINTER;
    }
    *str = (string_t*)malloc(sizeof(string_t));
    if (*str == NULL) {
        return err_num = NOT_ENOUGH_MEMORY;
    }
    (*str)->line = NULL;
    (*str)->size = 0;
    (*str)->capacity = 0;
    return err_num = SUCCESS_RETVAL;
}

void string_destroy(string_t* str) {
    free(str->line);
    free(str);
}

int string_push_back(string_t* str, char symbol) {
    if (str == NULL) {
        return err_num = INVALID_POINTER;
    }
    if (str->size >= str->capacity) {
        char* ptr;
        str->capacity += REALLOCATION_SIZE;
        ptr = (char*)realloc(str->line, str->capacity * sizeof(char));
        if (ptr == NULL) {
            return err_num = NOT_ENOUGH_MEMORY;
        }
        str->line = ptr;
    }
    str->line[str->size++] = symbol;
    return err_num = SUCCESS_RETVAL;
}
