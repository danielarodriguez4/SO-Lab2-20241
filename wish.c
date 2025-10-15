#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LINE 1024
#define MAX_PATHS 64
#define MAX_PARALLEL 64

char error_message[30] = "An error has occurred\n";
char *search_paths[MAX_PATHS];
int num_paths = 0;

void print_error() {
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void init_paths() {
    search_paths[0] = strdup("/bin");
    num_paths = 1;
}

char* find_executable(char *cmd) {
    static char full_path[256];
    for (int i = 0; i < num_paths; i++) {
        snprintf(full_path, sizeof(full_path), "%s/%s", search_paths[i], cmd);
        if (access(full_path, X_OK) == 0) {
            return full_path;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    FILE *input = stdin;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    init_paths();

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
            fflush(stdout);
        }
        read = getline(&line, &len, input);
        if (read == -1) {
            break;
        }
        
        // Eliminar salto de línea
        if (read > 0 && line[read - 1] == '\n') {
            line[read - 1] = '\0';
            read--;
        }
        
        // Ignorar líneas vacías
        if (read == 0 || strlen(line) == 0) {
            continue;
        }
        
        char *parallel_cmds[MAX_PARALLEL];
        int num_parallel = 0;
        char *line_copy = strdup(line);
        char *cmd_token = line_copy;
        char *amp_pos;
        
        while ((amp_pos = strchr(cmd_token, '&')) != NULL) {
            *amp_pos = '\0';
            parallel_cmds[num_parallel++] = strdup(cmd_token);
            cmd_token = amp_pos + 1;
        }
        parallel_cmds[num_parallel++] = strdup(cmd_token);
        free(line_copy);

        pid_t pids[MAX_PARALLEL];
        int num_pids = 0;

        // Ejecutar cada comando
        for (int cmd_idx = 0; cmd_idx < num_parallel; cmd_idx++) {
            char *cmd_line = parallel_cmds[cmd_idx];
            
            // Eliminar espacios al inicio y final
            while (*cmd_line == ' ' || *cmd_line == '\t') cmd_line++;
            if (strlen(cmd_line) == 0) continue;
            
            char *end = cmd_line + strlen(cmd_line) - 1;
            while (end > cmd_line && (*end == ' ' || *end == '\t')) {
                *end = '\0';
                end--;
            }
            if (strlen(cmd_line) == 0) continue;

            // Buscar redirección
            char *redir_file = NULL;
            char *redir_pos = strchr(cmd_line, '>');
            if (redir_pos != NULL) {
                *redir_pos = '\0';
                redir_file = redir_pos + 1;
                
                // Eliminar espacios del nombre del archivo
                while (*redir_file == ' ' || *redir_file == '\t') redir_file++;

                // Validar que no haya múltiples >
                if (strchr(redir_file, '>') != NULL) {
                    print_error();
                    goto cleanup_parallel;
                }
                
                // Verificar que solo haya un archivo
                char *redir_copy = strdup(redir_file);
                char *tok1 = redir_copy;
                while (*tok1 == ' ' || *tok1 == '\t') tok1++;
                char *tok2 = tok1;
                while (*tok2 && *tok2 != ' ' && *tok2 != '\t') tok2++;
                if (*tok2) {
                    *tok2++ = '\0';
                    while (*tok2 == ' ' || *tok2 == '\t') tok2++;
                    if (*tok2 != '\0') {
                        print_error();
                        free(redir_copy);
                        goto cleanup_parallel;
                    }
                }
                redir_file = strdup(tok1);
                free(redir_copy);
                
                if (strlen(redir_file) == 0) {
                    print_error();
                    goto cleanup_parallel;
                }
            }

            char *args[MAX_LINE];
            int argc_cmd = 0;
            char *cmd_copy = strdup(cmd_line);
            char *token;
            char *rest = cmd_copy;
            
            while ((token = strsep(&rest, " \t")) != NULL) {
                if (strlen(token) > 0) {
                    args[argc_cmd++] = token;
                }
            }
            args[argc_cmd] = NULL;

            if (argc_cmd == 0) {
                // Si hay redirección pero no hay comando, es un error
                if (redir_file != NULL) {
                    print_error();
                }
                free(cmd_copy);
                continue;
            }

            // Comando integrado: exit
            if (strcmp(args[0], "exit") == 0) {
                if (argc_cmd != 1) {
                    print_error();
                } else {
                    // Limpiar antes de salir
                    for (int i = 0; i < num_parallel; i++) {
                        free(parallel_cmds[i]);
                    }
                    free(cmd_copy);
                    exit(0);
                }
                free(cmd_copy);
                continue;
            }

            // Comando integrado: cd
            if (strcmp(args[0], "cd") == 0) {
                if (argc_cmd != 2) {
                    print_error();
                } else if (chdir(args[1]) != 0) {
                    print_error();
                }
                free(cmd_copy);
                continue;
            }

            // Comando integrado: path
            if (strcmp(args[0], "path") == 0) {
                // Liberar paths anteriores
                for (int i = 0; i < num_paths; i++) {
                    free(search_paths[i]);
                }
                num_paths = 0;
                
                // Agregar nuevos paths
                for (int i = 1; i < argc_cmd && i < MAX_PATHS; i++) {
                    search_paths[num_paths++] = strdup(args[i]);
                }
                free(cmd_copy);
                continue;
            }

            // Comando externo
            char *exec_path = find_executable(args[0]);
            if (exec_path == NULL) {
                print_error();
                free(cmd_copy);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                print_error();
                free(cmd_copy);
                continue;
            } else if (pid == 0) {
                // Proceso hijo
                
                // Manejar redirección
                if (redir_file != NULL) {
                    int fd = open(redir_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) {
                        print_error();
                        exit(1);
                    }
                    dup2(fd, STDOUT_FILENO);
                    dup2(fd, STDERR_FILENO);
                    close(fd);
                }
                
                // Construir array de argumentos para execv
                char *exec_args[MAX_LINE];
                for (int i = 0; i < argc_cmd; i++) {
                    exec_args[i] = args[i];
                }
                exec_args[argc_cmd] = NULL;
                
                execv(exec_path, exec_args);
                print_error();
                exit(1);
            } else {
                // Proceso padre
                pids[num_pids++] = pid;
            }
            
            free(cmd_copy);
        }

        // Esperar a que todos los procesos terminen
        for (int i = 0; i < num_pids; i++) {
            waitpid(pids[i], NULL, 0);
        }

cleanup_parallel:
        for (int i = 0; i < num_parallel; i++) {
            free(parallel_cmds[i]);
        }
    }
    free(line);
    if (input != stdin) {
        fclose(input);
    }
    return 0;
}
