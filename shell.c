#define _POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

#include "readline.h"
#include "replace.h"
#include "lib_string.h"
#include "arguments_t.h"
#include "error.h"

#define NONE 0
#define SINGLE_QUOTE 1
#define DOUBLE_QUOTE 2
#define BACKSLASH 3
#define INPUT_FILE 4
#define OUTPUT_FILE 5

#define TRUE 1
#define FALSE 0

#define MAX_JOBS_COUNT 1024

int current_job = 0;
char* pwd;
char* string;
pid_t child;

struct job {
    int id;
    char* cmd;
    pid_t pid;
} jobs[MAX_JOBS_COUNT];

int string_copy(char** target, char* source) {
    char* ptr = (char*)realloc(*target, (strlen(source) + 1) * sizeof(char));
    if (ptr == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    strcpy(ptr, source);
    *target = ptr;
}

char* full_path(char* str) {
    int size = 0;
    char slash = (str[0] == '/');
    char* file = strtok(str, "/");
    char* res = (char*)malloc((strlen(pwd) + strlen(str) + 2) * sizeof(char));
    if (res == NULL) {
        fprintf(stderr, "failed to allocate memory\n");
        exit(EXIT_FAILURE);
    }
    if (!slash && file != NULL) {
        if (!strcmp(file, "~")) {
            strcpy(res, getenv("HOME"));
            size += strlen(getenv("HOME"));
            file = strtok(NULL, "/");
        } else {
            strcpy(res, pwd);
            size += strlen(pwd);
            if (!strcmp(file, "..")) {
                while (size > 0 && res[--size] != '/')
                    ;
                res[size] = '\0';
                file = strtok(NULL, "/");
            } else if (!strcmp(file, ".")) {
                file = strtok(NULL, "/");
            }
        }
    }
    while (file != NULL) {
        if (!strcmp(file, "..")) {
            while (size > 0 && res[--size] != '/')
                ;
            res[size] = '\0';
        } else if (strcmp(file, ".")) {
            strcat(res, "/");
            strcat(res, file);
            size += (1 + strlen(file));
        }
        file = strtok(NULL, "/");
    }
    if (!size) {
        strcpy(res, "/");
    }
    return res;
}

char inline_command(char** argv, int output) {
    if (!strcmp(argv[0], "exit")) {
        exit(EXIT_SUCCESS);
    }
    if (!strcmp(argv[0], "cd")) {
        char *temp = NULL;
        if (argv[1] == NULL || !strcmp(argv[1], "~")) {
            string_copy(&temp, getenv("HOME"));
        } else {
            temp = full_path(argv[1]);
        }
        if (0 == chdir(temp)) {
            free(pwd);
            pwd = temp;
        } else {
            fprintf(stderr, "%s: no such directory\n", temp);
            free(temp);
        }
        return TRUE;
    }
    child = fork();
    if (-1 == child) {
        fprintf(stderr, "%s: failed to create process\n", argv[0]);
    } else if (0 == child) {
        if (-1 != output) {
            dup2(output, STDOUT_FILENO);
            close(output);
        }
        if (!strcmp(argv[0], "pwd")) {
            printf("%s\n", pwd);
            _exit(EXIT_SUCCESS);
        }
        if (!strcmp(argv[0], "jobs")) {
            int index;
            for (index = 0; index < current_job; ++index) {
                printf("[%d] %s\n", jobs[index].id, jobs[index].cmd);
            }
            _exit(EXIT_SUCCESS);
        }
        if (!strcmp(argv[0], "bg")) {
            int job_index;
            if (argv[1] == NULL) {
                if (!current_job) {
                    fprintf(stderr, "bg: no current job\n");
                    _exit(EXIT_SUCCESS);
                }
                job_index = current_job - 1;
            } else {
                char* end_ptr;
                job_index = strtol(argv[1], &end_ptr, 10);
                if (*argv[1] == '\0' || *end_ptr != '\0') {
                    fprintf(stderr, "usage: bg <job number>\n");
                    _exit(EXIT_SUCCESS);
                }
            }
            kill(jobs[job_index].pid, SIGCONT);
            printf("[%d] %s &\n", jobs[job_index].id, jobs[job_index].cmd);
            _exit(EXIT_SUCCESS);
        }
        if (!strcmp(argv[0], "msed")) {
            int size = 1;
            while (argv[size] != NULL)
                ++size;
            if (size == 4) {
                REPLACE_HANDLER(replace(&argv[1], argv[2], argv[3]));
                printf("%s\n", argv[1]);
            } else {
                fprintf(stderr, "usage: msed <text> <pattern> <target>\n");
            }
            _exit(EXIT_SUCCESS);
        }
        if (!strcmp(argv[0], "mcat")) {
            FILE* fp = stdin;
            if (argv[1] != NULL) {
                if ((fp = fopen(argv[1], "r")) == NULL) {
                    fprintf(stderr, "%s: no such file\n", argv[1]);
                    _exit(EXIT_FAILURE);
                }
            }
            while (1) {
                int retval;
                char* string;
                if (retval = readline(fp, &string, '\n')) {
                    if (retval == END_OF_FILE) {
                        break;
                    }
                    print_error(argv[0]);
                }
                printf("%s\n", string);
                free(string);
            }
            _exit(EXIT_SUCCESS);
        }
        _exit(EXIT_FAILURE);
    } else {
        int result;
        waitpid(child, &result, 0);
        child = -1;
        return !result;
    }
}

void external_command(char** argv, int input, int output) {
    child = fork();
    if (-1 == child) {
        fprintf(stderr, "%s: failed to create process\n", argv[0]);
    } else if (0 == child) {
        if (-1 != input) {
            dup2(input, STDIN_FILENO);
            close(input);
        }
        if (-1 != output) {
            dup2(output, STDOUT_FILENO);
            close(output);
        }
        if (-1 == execvp(*argv, argv)) {
            fprintf(stderr, "%s: command not found\n", argv[0]);
            _exit(EXIT_FAILURE);
        }
    } else {
        waitpid(child, NULL, 0);
        child = -1;
    }
}

void find_executable(char** argv, int input, int output) {
    if (!inline_command(argv, output)) {
        external_command(argv, input, output);
    }
    if (-1 != input) {
        close(input);
    }
    if (-1 != output) {
        close(output);
    }
}

inline char isdelim(char c, char* str) {
    do {
        if (*str == c)
            return TRUE;
    } while (*str++ != '\0');
    return FALSE;
}

void split_into_commands(char **argv, char* line) {
    char mask = NONE;
    char* ptr = line;
    int input = -1;
    int output = -1;

    arguments_t* args;
    if (arguments_create(&args)) {
        print_error(argv[0]);
    }

    while (1) {
        if (mask == NONE) {
            if (*ptr == '\\') {
                mask = BACKSLASH;
            } else if (*ptr == '\'') {
                mask = SINGLE_QUOTE;
            } else if (*ptr == '\"') {
                mask = DOUBLE_QUOTE;
            } else if (isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (args->opt_size > 0) {
                    if (arguments_push_back(args, '\0')) {
                        print_error(argv[0]);
                    }
                }
                if (*ptr == '<') {
                    mask = INPUT_FILE;
                } else if (*ptr == '>') {
                    mask = OUTPUT_FILE;
                } else if (isdelim(*ptr, "|;#\0")) {
                    char boo = FALSE;
                    int pdes[2];
                    if (*ptr == '|') {
                        if (-1 == pipe(pdes)) {
                            fprintf(stderr, "%s: failed to create pipe\n", argv[0]);
                        } else if (output == -1) {
                            output = pdes[1];
                            boo = TRUE;
                        }
                    }
                    if (args->size > 0) {
                        args->options[args->size] = NULL;
                        find_executable(args->options, input, output);
                        arguments_destroy(args);
                        if (arguments_create(&args)) {
                            print_error(argv[0]);
                        }
                    }
                    input = output = -1;
                    if (boo) {
                        input = pdes[0];
                    }
                    if (isdelim(*ptr, "#\0")) {
                        break;
                    }
                }
            } else {
                if (arguments_push_back(args, *ptr)) {
                    print_error(argv[0]);
                }
            }
        } else if (mask == BACKSLASH) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near backslash\n", argv[0]);
                break;
            } else {
                if (arguments_push_back(args, *ptr)) {
                    print_error(argv[0]);
                }
            }
            mask = NONE;
        } else if (mask == SINGLE_QUOTE) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near single quote\n", argv[0]);
                break;
            } else if (*ptr == '\'') {
                mask = NONE;
            } else {
                if (arguments_push_back(args, *ptr)) {
                    print_error(argv[0]);
                }
            }
        } else if (mask == DOUBLE_QUOTE) {
            if (*ptr == '\0') {
                fprintf(stderr, "%s: error near double quote\n", argv[0]);
                break;
            } else if (*ptr == '\"') {
                mask = NONE;
            //} else if (*ptr == '$') {
            } else {
                if (arguments_push_back(args, *ptr)) {
                    print_error(argv[0]);
                }
            }
        } else if (mask == INPUT_FILE) {
            string_t* file;
            if (string_create(&file)) {
                print_error(argv[0]);
            }
            while (isdelim(*ptr, " \f\r\t\v")) {
                ++ptr;
            }
            while (!isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (string_push_back(file, *ptr)) {
                    print_error(argv[0]);
                }
                ++ptr;
            }
            if (0 == file->size) {
                fprintf(stderr, "input stream: syntax error\n");
                break;
            }
            string_push_back(file, '\0');
            if ((input = open(file->line, O_RDONLY, 0644)) == -1) {
                fprintf(stderr, "%s: no such file\n", file->line);
                break;
            }
            string_destroy(file);
            --ptr;
            mask = NONE;
        } else if (mask == OUTPUT_FILE) {
            int mode = O_WRONLY|O_CREAT;
            string_t* file;
            if (string_create(&file)) {
                print_error(argv[0]);
            }
            if (*ptr == '>') {
                mode |= O_APPEND;
                ++ptr;
            } else {
                mode |= O_TRUNC;
            }
            while (isdelim(*ptr, " \f\r\t\v")) {
                ++ptr;
            }
            while (!isdelim(*ptr, "<>|;# \f\r\t\v\0")) {
                if (string_push_back(file, *ptr)) {
                    print_error(argv[0]);
                }
                ++ptr;
            }
            if (0 == file->size) {
                fprintf(stderr, "output stream: syntax error\n");
                break;
            }
            string_push_back(file, '\0');
            if ((output = open(file->line, mode, 0644)) == -1) {
                fprintf(stderr, "%s: failed to open file\n", file->line);
                break;
            }
            string_destroy(file);
            --ptr;
            mask = NONE;
        }
        ++ptr;
    }
    arguments_destroy(args);
}

void signal_handler(int signum) {
    if (signum == SIGTSTP) {
        if (child > 0) {
            int len = strlen(string);
            kill(child, SIGTSTP);
            jobs[current_job].id = current_job + 1;
            jobs[current_job].cmd = (char*)malloc(len * sizeof(char));
            strncpy(jobs[current_job].cmd, string, len);
            jobs[current_job].pid = child;
            printf("\n[%d] %s\n", jobs[current_job].id, jobs[current_job].cmd);
            ++current_job;
        } else {
            kill(getpid(), SIGTSTP);
            printf("\n");
        }
    } else if (signum == SIGINT) {
        if (child > 0) {
            kill(child, SIGINT);
        } else {
            kill(getpid(), SIGINT);
        }
        printf("\n");
    }
}

int main(int argc, char** argv) {
    char* invitation = "$ ";

    signal(SIGTSTP, signal_handler);
    signal(SIGINT, signal_handler);

    string_copy(&pwd, getenv("PWD"));

    while (1) {
        int retval;
        printf("%s", invitation);
        if (retval = readline(stdin, &string, '\n')) {
            if (retval == END_OF_FILE) {
                printf("\n");
                break;
            }
            print_error(argv[0]);
        }
        split_into_commands(argv, string);
        free(string);
    }

    return 0;
}
