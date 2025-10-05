#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
char error_message[30] = "An error has occurred\n";

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    // Modo batch
    if (argc == 2) {
        input = fopen(argv[1], "r");
        if (!input) {
            print_error();
            exit(1);
        }
    } else if (argc > 2) {
        print_error();
        exit(1);
    }

    while (1) {
        if (input == stdin) {
            printf("wish> ");
        }
        read = getline(&line, &len, input);
        if (read == -1) {
            break;
        }
        // Eliminar salto de línea
        if (line[read - 1] == '\n') {
            line[read - 1] = '\0';
        }
        // Ignorar líneas vacías
        if (strlen(line) == 0) {
            continue;
        }
        // Parsear comando y argumentos
        char *args[MAX_LINE];
        int argc_cmd = 0;
        char *token = strtok(line, " ");
        while (token != NULL) {
            args[argc_cmd++] = token;
            token = strtok(NULL, " ");
        }
        args[argc_cmd] = NULL;
        // Comando integrado: exit
        if (strcmp(args[0], "exit") == 0) {
            if (argc_cmd != 1) {
                print_error();
                continue;
            }
            exit(0);
        }
        // Comando integrado: cd
        if (strcmp(args[0], "cd") == 0) {
            if (argc_cmd != 2) {
                print_error();
                continue;
            }
            if (chdir(args[1]) != 0) {
                print_error();
            }
            continue;
        }
        // Comando externo
        pid_t pid = fork();
        if (pid < 0) {
            print_error();
            continue;
        } else if (pid == 0) {
            // Proceso hijo
            char path[256] = "/bin/";
            strncat(path, args[0], sizeof(path) - strlen(path) - 1);
            char *exec_args[MAX_LINE];
            for (int i = 0; i < argc_cmd; i++) {
                exec_args[i] = args[i];
            }
            exec_args[argc_cmd] = NULL;
            execv(path, exec_args);
            print_error();
            exit(1);
        } else {
            // Proceso padre
            wait(NULL);
        }
    }
    free(line);
    if (input != stdin) {
        fclose(input);
    }
    return 0;
}
