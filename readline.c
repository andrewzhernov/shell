#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

/* Размер страницы памяти */
#define MEMORY_PAGE_SIZE sysconf(_SC_PAGESIZE)

#define CHECK_RES(state, msg) do { \
    if (state) { \
        fprintf(stderr, msg); \
        exit(0); \
    } \
} while (0); \

/* Безопасное считывание строки */
void readline(char **line, FILE *pFile) {
    size_t size = 0;
    size_t capacity = MEMORY_PAGE_SIZE;
    int symbol;

    CHECK_RES(line == NULL, "readline: invalid pointer\n");
    CHECK_RES(pFile == NULL, "readline: can't open a file\n");

    *line = (char*)malloc(capacity * sizeof(char));
    CHECK_RES(*line == NULL, "readline: failed to allocate memory\n");

    while ((symbol = fgetc(pFile)) != '\n' && symbol != EOF) {
        if (size + 1 >= capacity) {
            capacity += MEMORY_PAGE_SIZE;
            *line = (char*)realloc(*line, capacity * sizeof(char));
            CHECK_RES(*line == NULL, "readline: failed to allocate memory\n");
        }
        (*line)[size++] = symbol;
    }
    (*line)[size] = '\0';

    CHECK_RES(symbol == EOF, "\n");
    CHECK_RES(ferror(pFile), "readline: file error\n");
}
