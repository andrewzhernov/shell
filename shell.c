#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define ARGS_COUNT 8

#define CHECK_PTR(ptr) { \
    if (ptr == NULL) { \
        fprintf(stderr, "shell: failed to allocate memory"); \
        exit(1); \
    } \
} \

char *pwd;

char inline_command(char** argv) {
    if (!strcmp(argv[0], "exit")) {
        exit(0);
    }
    if (!strcmp(argv[0], "pwd")) {
        printf("%s\n", pwd);
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
    if (0 == child) {
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
    } else if (-1 == child) {
        fprintf(stderr, "%s: failed to create process\n", *argv);
    } else {
        waitpid(child, NULL, 0);
    }
}

void find_executable(char** argv, char *input, char *output, char append) {
    if (!inline_command(argv)) {
        external_command(argv, input, output, append);
    }
}

void streams_handler(char** argv) {
    int size = ARGS_COUNT, i = 0, last = 0;
    char append;
    char *input = NULL, *output = NULL;
    char **new_argv = (char**)malloc(size * sizeof(char*));

    for (i = 0; argv[i] != NULL; ++i) {
        if (argv[i][0] == '<') {
            if (argv[i][1] != '\0') {
                input = &argv[i][1];
            } else if (argv[++i] != NULL) {
                input = argv[i];
            } else {
                fprintf(stderr, "operator <: syntax error\n");
                exit(1);
            }
        } else if (argv[i][0] == '>') {
            if (argv[i][1] == '>') {
                if (argv[i][2] != '\0') {
                    output = &argv[i][2];
                } else if (argv[++i] != NULL) {
                    output = argv[i];
                } else {
                    fprintf(stderr, "operator >>: syntax error\n");
                    exit(1);
                }
                append = TRUE;
            } else {
                if (argv[i][1] != '\0') {
                    output = &argv[i][1];
                } else if (argv[++i] != NULL) {
                    output = argv[i];
                } else {
                    fprintf(stderr, "operator >: syntax error\n");
                    exit(1);
                }
                append = FALSE;
            }
        } else {
            if (last + 1 >= size) {
                size += ARGS_COUNT;
                new_argv = (char**)realloc(new_argv, size * sizeof(char*));
                CHECK_PTR(new_argv);
            }
            new_argv[last++] = argv[i];
        }
    }
    new_argv[last] = NULL;
    find_executable(new_argv, input, output, append);
}

void split_into_commands(char **args, char* input) {
    int i, j;
    int size = strlen(input);
    char s_quote = FALSE;
    char d_quote = FALSE;
    int start = 0;
    int last = 0;

    size_t capacity = ARGS_COUNT;
    char** argv = (char**)malloc(capacity * sizeof(char*));
    CHECK_PTR(argv);

    for (i = 0; i <= size; ++i) {
        if (s_quote || d_quote) {
            if (input[i] == '\'' && s_quote) {
                s_quote = FALSE;
            } else if (input[i] == '\"' && d_quote) {
                d_quote = FALSE;
            } else if (i == size) {
                fprintf(stderr, "%s: error near quote", args[0]);
                exit(1);
            }
        } else {
            if (input[i] == '#') {
                break;            
            } else if (input[i] == '\\') {
                ++i;
                if (input[i] == '\0') {
                    fprintf(stderr, "%s: error near backslash\n", args[0]);
                    exit(1);
                }
            } else if (input[i] == '\'') {
                s_quote = TRUE;
            } else if (input[i] == '\"') {
                d_quote = TRUE;
            } else if (isspace(input[i]) || input[i] == ';' || i == size) {
                if (i != start) {
                    if (last + 1 >= capacity) {
                        capacity += ARGS_COUNT;
                        argv = (char**)realloc(argv, capacity * sizeof(char*));
                        CHECK_PTR(argv);
                    }
                    argv[last] = (char*)malloc((i - start + 1) * sizeof(char));
                    CHECK_PTR(argv[last]);
                    for (j = start; j < i; ++j)
                        argv[last][j - start] = input[j];
                    argv[last][i - start] = '\0';
                    ++last;
                }
                start = i + 1;

                if (last > 0 && (input[i] == ';' || i == size)) {
                    argv[last] = NULL;
                    streams_handler(argv);

                    for (j = 0; argv[j] != NULL; ++j)
                        free(argv[j]);

                    last = 0;
                    capacity = ARGS_COUNT;
                    argv = (char**)realloc(argv, capacity * sizeof(char*));
                    CHECK_PTR(argv);
                }
            }
        }
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
