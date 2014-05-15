#include "arguments_t.h"

int arguments_create(arguments_t** args) {
    if (args == NULL) {
        return err_num = INVALID_POINTER;
    }
    *args = (arguments_t*)malloc(sizeof(arguments_t));
    if (*args == NULL) {
        return err_num = NOT_ENOUGH_MEMORY;
    }
    (*args)->options = NULL;
    (*args)->size = 0;
    (*args)->capacity = 0;
    (*args)->opt_size = 0;
    (*args)->opt_capacity = 0;
    return err_num = SUCCESS_RETVAL;
}

void arguments_destroy(arguments_t* args) {
    int index;
    for (index = 0; index < args->size; ++index)
        free(args->options[index]);
    free(args->options);
    free(args);
}

int arguments_push_back(arguments_t* args, char symbol) {
    if (args == NULL) {
        return err_num = INVALID_POINTER;
    }
    if (args->size >= args->capacity) {
        char** ptr;
        if (!args->capacity) {
            args->options = NULL;
        }
        args->capacity += REALLOCATION_SIZE;
        ptr = (char**)realloc(args->options, args->capacity * sizeof(char*));
        if (ptr == NULL) {
            return err_num = NOT_ENOUGH_MEMORY;
        }
        args->options = ptr;
    }
    if (args->opt_size >= args->opt_capacity) {
        char* ptr;
        if (!args->opt_capacity) {
            args->options[args->size] = NULL;
        }
        args->opt_capacity += REALLOCATION_SIZE;
        ptr = (char*)realloc(args->options[args->size], args->opt_capacity * sizeof(char));
        if (ptr == NULL) {
            return err_num = NOT_ENOUGH_MEMORY;
        }
        args->options[args->size] = ptr;
    }
    args->options[args->size][args->opt_size++] = symbol;
    if (symbol == '\0') {
        ++args->size;
        args->opt_size = 0;
        args->opt_capacity = 0;
    }
    return err_num = SUCCESS_RETVAL;
}
