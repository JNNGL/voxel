#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int readline(char* buf, size_t size) {
    size_t read = 0;
    while (read < size - 1) {
        char readbuf[2];
        size_t nread = fread(readbuf, 1, 1, stdin);
        if (nread > 0) {
            buf[read] = readbuf[0];
            if (buf[read] == '\n') {
                buf[++read] = 0;
                return read;
            }
            ++read;
        }
    }
    buf[read] = 0;
    return read;
}

int main(int argc, char* const argv[]) {
    puts("sh - version 1.0.0-dev");

    char working_dir[1024] = {'/', 0};
    while (1) {
        printf("%s %% ", working_dir);
        fflush(stdout);

        char command[1024];
        readline(command, 1024);
        command[strlen(command) - 1] = 0;

        char* p;
        char* tokens[512];
        char* last;

        int i = 0;
        for ((p = strtok_r(command, " ", &last)); p; (p = strtok_r(NULL, " ", &last)), i++) {
            if (i < 511) {
                tokens[i] = p;
            }
        }

        tokens[i] = NULL;

        if (!tokens[0] || strlen(tokens[0]) < 1) {
            continue;
        }

        if (!strcmp(tokens[0], "exit")) {
            break;
        }

        char* command_ptr = command;
        if (access(tokens[0], 0)) {
            if (!strstr(tokens[0], "/")) {
                static char _command[1024];
                snprintf(_command, 1024, "/bin/%s", tokens[0]);
                command_ptr = _command;
                if (access(_command, 0)) {
                    printf("sh: command not found: %s\n", tokens[0]);
                    continue;
                }
            } else {
                printf("sh: command not found: %s\n", tokens[0]);
                continue;
            }
        }

        pid_t child = fork();
        if (child == 0) {
            return execv(command_ptr, tokens);
        } else {
            syscall_waitpid(child);
        }
    }

    return 0;
}