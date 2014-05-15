#include "readline.h"

int readline(FILE* pFile, char** line, char delim) {
    size_t size = 0;
    size_t capacity = 0;
    int symbol;

    if (line == NULL) {
        return err_num = INVALID_POINTER;
    }
    if (pFile == NULL) {
        return err_num = ERROR_OPENING_FILE;
    }

    *line = NULL;
    while ((symbol = fgetc(pFile)) != delim && symbol != EOF) {
        if (size + 1 >= capacity) {
            char* ptr;
            capacity += MEMORY_PAGE_SIZE;
            if ((ptr = (char*)realloc(*line, capacity * sizeof(char))) == NULL) {
                return err_num = NOT_ENOUGH_MEMORY;
            }
            *line = ptr;
        }
        (*line)[size++] = symbol;
    }
    if (ferror(pFile)) {
        free(*line);
        *line = NULL;
        return err_num = FILE_ERROR;
    }
    if (!size) {
        *line = malloc(sizeof(char));
    }
    (*line)[size] = '\0';
    
    if (symbol == EOF) {
        return err_num = END_OF_FILE;
    }
    return err_num = SUCCESS_RETVAL;
}
