#include "error.h"

int err_num;
const char* err_messages[] = {
    "success",
    "failed to allocate memory",
    "invalid pointer",
    "can't open a file",
    "failed to read from a file stream"
};

void print_error(const char* msg) {
    if (msg != NULL && *msg != '\0') {
        fprintf(stderr, "%s: ", msg);
    }
    fprintf(stderr, "%s\n", err_messages[err_num]);
    exit(EXIT_FAILURE);
}
