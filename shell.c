#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {

    while (1) {

        char *cmd_line = NULL; // ptr pra a string     |
        size_t len = 0;        // pega o tamanho dela  | getline
        ssize_t read;          // var q vai receber    |

        pid_t pid;

        printf("vfpa> ");
        fflush(stdout);

        read = getline(&cmd_line, &len, stdin);

        if (read == -1) {
            printf("erro na leitura da linha");
        }

        cmd_line[strcspn(cmd_line, "\n")] = '\0'; // troca o \n por \0

        if (strcmp(cmd_line, "exit") == 0) { // da exit
            free(cmd_line);
            exit(0);
        }

        char *token; // strtok & recebe strings sem limitacao
        char **args_execvp = (char **)malloc((len + 1) * sizeof(char *)); // aloca de acordo com o tam da cmd line

        if (args_execvp == NULL) {
            perror("Erro na alocacao de memoria");
            return 1;
        }

        int flag_semicolon = 0; // muda do sequencial simples para o com ;

        for (int i = 0; cmd_line[i] != '\0'; i++) { // verifica se são varias sequenciais, ou seja, se tem ;
            if (cmd_line[i] == ';') {
                flag_semicolon = 1;
                break;
            }
        }

        // printf("flag %d", flag_semicolon);

        if (flag_semicolon == 1) { // COM ;

            char **args_execvp_semicolon = (char **)malloc((len + 1) * sizeof(char *)); // recebe comandos ate ;
            int c1 = 0;

        token = strtok(cmd_line, ";"); // separa em tokens
            while (token != NULL) {
                args_execvp_semicolon[c1] = token;
                token = strtok(NULL, ";");
                c1++;
            }
            args_execvp_semicolon[c1] = NULL; // coloca null no fim do comando

            for (int i = 0; args_execvp_semicolon[i] != NULL; i++) {
            
                int c2 = 0;
                char *aes = args_execvp_semicolon[i];

                token = strtok(aes, " "); // separa em tokens
                while (token != NULL) {
                    args_execvp[c2] = token;
                    token = strtok(NULL, " ");
                    c2++;
                }
                args_execvp[c2] = NULL; // coloca null no fim do comando

                pid = fork(); // cria um processo filho

                if (pid < 0) {
                    fprintf(stderr, "Fork Failed");
                    free(cmd_line);
                    free(args_execvp);
                    return 1;
                } else if (pid == 0) { // se for o processo filho, executa
                    execvp(args_execvp[0], args_execvp);
                    perror("execvp");
                    exit(0);
                } else { // se nao, eh o pai, fica em wait
                    wait(NULL);
                }
            }

            free(args_execvp_semicolon);


        } else { // SEM ;

            int c = 0;
            token = strtok(cmd_line, " "); // separa em tokens
            while (token != NULL) {
                args_execvp[c] = token;
                token = strtok(NULL, " ");
                c++;
            }
            args_execvp[c] = NULL; // coloca null no fim do comando

            pid = fork(); // cria um processo filho

            if (pid < 0) {
                fprintf(stderr, "Fork Failed");
                free(cmd_line);
                free(args_execvp);
                return 1;

            } else if (pid == 0) { // se for o processo filho, executa
                execvp(args_execvp[0], args_execvp);
                perror("execvp");
                exit(0);

            } else { // se nao, eh o pai, fica em wait
                wait(NULL);
            }
        }
        free(args_execvp); // libera argumentos
        free(cmd_line);    // libera a linha alocada pelo getline
    }

  return 0;
}

// precisa corrigir
// se colocar mts espaços antes do comando, remover
// exit funcionar se tiver no ;
// arrumar o seq pra rodar todos os comandos num mesmo processo, e nao encerrar
