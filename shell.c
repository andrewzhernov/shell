#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "readline.h"

#define NONE 0
#define SINGLE_QUOTE 1
#define DOUBLE_QUOTE 2
#define BACKSLASH 3
#define STREAM 4
#define INPUT 5
#define OUTPUT 6

#define TRUE 1
#define FALSE 0

#define ARGS_COUNT 4
#define ARG_SIZE 4

#define CHECK_PTR(ptr) { \
    if (ptr == NULL) { \
        fprintf(stderr, "shell: failed to allocate memory\n"); \
        return; \
    } \
} \

char *pwd;

char inline_command(char** argv, char* output, char append) {
    if (!strcmp(argv[0], "exit")) {
        exit(0);
    }
    if (!strcmp(argv[0], "pwd")) {
        if (output != NULL) {
            FILE* pFile = fopen(output, (append ? "a": "w"));
            fprintf(pFile, "%s\n", pwd);
            fclose(pFile);
        } else {
            printf("%s\n", pwd);
        }
        return TRUE;
    }
    if (!strcmp(argv[0], "cd")) {
        char *temp;
        if (argv[1] == NULL) {
            temp = getenv("HOME");
        } else {
            temp = argv[1];
        }
        if (0 == chdir(temp)) {
            pwd = (char*)realloc(pwd, strlen(temp) + 1);
            CHECK_PTR(pwd);
            strcpy(pwd, temp);
            printf("current directory: %s\n", pwd);
        } else {
            fprintf(stderr, "%s: no such directory\n", temp);
        }
        return TRUE;
    }
    return FALSE;
}

void external_command(char **argv, char *input, char *output, char append) {
    pid_t child = fork();
    if (-1 == child) {
        fprintf(stderr, "%s: failed to create process\n", *argv);
    } else if (0 == child) {
        if (input != NULL) {
            int fd = open(input, O_RDONLY, 0644);
            if (-1 == fd) {
                fprintf(stderr, "%s: no such file or directory\n", input);
                _exit(0);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (output != NULL) {
            int fd, mode = O_WRONLY|O_CREAT|O_TRUNC;
            if (append) {
                mode |= O_APPEND;
            }
            fd = open(output, mode, 0644);
            if (-1 == fd) {
                fprintf(stderr, "%s: no such file or directory\n", output);
                _exit(0);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (-1 == execvp(*argv, argv)) {
            fprintf(stderr, "%s: command not found\n", *argv);
            _exit(0);
        }
    } else {
        waitpid(child, NULL, 0);
    }
}

void find_executable(char** argv, char *input, char *output, char append) {
    if (!inline_command(argv, output, append)) {
        external_command(argv, input, output, append);
    }
}

inline char isdelim(char c, char *str) {
    do {
        if (*str == c)
            return TRUE;
    } while (*str++ != '\0');
    return FALSE;
}

void split_into_commands(char **argv, char* line) {
    char mask = NONE;
    char *ptr = line;
    char *input = NULL;
    char *output = NULL;
    char** args = NULL;
    char* option = NULL;
    size_t last = 0;
    size_t index = 0;
    size_t capacity = 0;
    size_t size = 0;
    char append = FALSE;

    while (1) {
        if (mask == NONE) {
            if (*ptr == '\\') {
                mask = BACKSLASH;
            } else if (*ptr == '\'') {
                mask = SINGLE_QUOTE;
            } else if (*ptr == '\"') {
                mask = DOUBLE_QUOTE;
            } else if (isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (index) {
                    if (last + 1 >= capacity) {
                        capacity += ARGS_COUNT;
                        args = (char**)realloc(args, capacity * sizeof(char*));
                        CHECK_PTR(args);
                    }
                    option[index] = '\0';
                    args[last++] = option;
                    index = 0;
                    size = 0;
                    option = NULL;
                }
                if (*ptr == '<') {
                    mask = INPUT;
                } else if (*ptr == '>') {
                    mask = OUTPUT;
                }
                if (isdelim(*ptr, "|;#\0")) {
                    if (last > 0) {
                        args[last] = NULL;
                        find_executable(args, input, output, append);
                        for (index = 0; index < last; ++index)
                            free(args[index]);
                        free(args);
                        last = 0;
                        capacity = 0;
                        args = NULL;
                    }
                    free(input);
                    free(output);
                    input = output = NULL;
                    if (isdelim(*ptr, "#\0")) {
                        break;
                    }
                }
            } else {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    option = (char*)realloc(option, size * sizeof(char));
                    CHECK_PTR(option);
                }
                option[index++] = *ptr;
            }
        } else if (mask == BACKSLASH) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near backslash\n", argv[0]);
                break;
            } else {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    option = (char*)realloc(option, size * sizeof(char));
                    CHECK_PTR(option);
                }
                option[index++] = *ptr;
            }
            mask = NONE;
        } else if (mask == SINGLE_QUOTE) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near single quote\n", argv[0]);
                break;
            } else if (*ptr == '\'') {
                mask = NONE;
            } else {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    option = (char*)realloc(option, size * sizeof(char));
                    CHECK_PTR(option);
                }
                option[index++] = *ptr;
            }
        } else if (mask == DOUBLE_QUOTE) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near double quote\n", argv[0]);
                break;
            } else if (*ptr == '\"') {
                mask = NONE;
            //} else if (*ptr == '$') {
            } else {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    option = (char*)realloc(option, size * sizeof(char));
                    CHECK_PTR(option);
                }
                option[index++] = *ptr;
            }
        } else if (mask == INPUT) {
            while (isdelim(*ptr, " \f\r\t\v"))
                ++ptr;
            while (!isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    input = (char*)realloc(input, size * sizeof(char));
                    CHECK_PTR(input);
                }
                input[index++] = *ptr;
                ++ptr;
            }
            if (!index) {
                fprintf(stderr, "input stream '<': syntax error\n");
                break;
            }
            input[index] = '\0';
            index = 0;
            size = 0;
            --ptr;
            mask = NONE;
        } else if (mask == OUTPUT) {
            if (*ptr == '>') {
                append = TRUE;
                ++ptr;
            } else {
                append = FALSE;
            }
            while (isdelim(*ptr, " \f\r\t\v"))
                ++ptr;
            while (!isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (index + 1 >= size) {
                    size += ARG_SIZE;
                    output = (char*)realloc(output, size * sizeof(char));
                    CHECK_PTR(output);
                }
                output[index++] = *ptr;
                ++ptr;
            }
            if (!index) {
                fprintf(stderr, "output stream '%s': syntax error\n", (append ? ">>" : ">"));
                break;
            }
            output[index] = '\0';
            index = 0;
            size = 0;
            --ptr;
            mask = NONE;
        }
        ++ptr;
    }
}

int main(int argc, char **argv) {
    char *input;
    char *invitation = "$ ";
    pwd = (char*)malloc((strlen(getenv("PWD")) + 1) * sizeof(char));
    CHECK_PTR(pwd);
    strcpy(pwd, getenv("PWD"));

    while (1) {
        printf("%s", invitation);
        readline(&input, stdin);
        split_into_commands(argv, input);
        free(input);
    }

    return 0;
}
