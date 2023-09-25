#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

int sequential_style(char *cmd_line, size_t len, ssize_t read);
int batch(char *FILENAME);
int parallel_style(char *cmd_line, size_t len, ssize_t read);
void *execute_parallel(void *arg);

// argc - qtd de args q vao passar
// argv indice 0 eh o inicio do programa
// se a argc = 1, vai rodar normal, se argc == 2, roda batch

int main(int argc, char *argv[]) {

    int style_flag = 0; // indicar se eh parallel, sequential

    char *cmd_line = NULL; // ptr pra a string     |
    size_t len = 0;        // pega o tamanho dela  | getline
    ssize_t read;          // var q vai receber    |

    while (1) {

        if(argc == 1){ // se so tiver um arg, eh seq/pal

            if(style_flag == 0){
                printf("vfpa seq> ");
                fflush(stdout);

                read = getline(&cmd_line, &len, stdin);

                if (read == -1) {
                    printf("Erro na leitura da linha\n");
                    return 1;
                }

                if (strcmp(cmd_line, "\n") == 0){
                    continue;
                }

                cmd_line[strcspn(cmd_line, "\n")] = '\0'; // troca o \n por \0

                if (strcmp(cmd_line, "exit") == 0) { // da exit
                    free(cmd_line);
                    cmd_line = NULL;
                    exit(0);
                }

                if(strcmp(cmd_line, "style parallel") == 0){
                    style_flag = 1;
                    continue;
                }
                sequential_style(cmd_line, len, read);
            
            }else if(style_flag == 1){
                printf("vfpa par> ");
                fflush(stdout);
            
                read = getline(&cmd_line, &len, stdin);

                if (read == -1) {
                    printf("Erro na leitura da linha\n");
                    return 1;
                }

                if (strcmp(cmd_line, "\n") == 0){
                    continue;
                }

                cmd_line[strcspn(cmd_line, "\n")] = '\0'; // troca o \n por \0

                if (strcmp(cmd_line, "exit") == 0) { // da exit
                    free(cmd_line);
                    cmd_line = NULL;
                    exit(0);
                }

                if(strcmp(cmd_line, "style sequential") == 0){
                    style_flag = 0;
                    continue;
                }

                parallel_style(cmd_line, len, read);
            
            }

        }else if(argc == 2){ // batch mode

            // char *linha = NULL;
            // size_t tamanho = 0;
            // ssize_t bytes_lidos;

            // char *nomeArquivo = argv[1];
            // FILE *arquivo = fopen(nomeArquivo, "r"); // Abre o arquivo para leitura

            // if (arquivo == NULL) {
            //     perror("Erro ao abrir o arquivo");
            //     return 1;
            // }

            // while ((bytes_lidos = getline(&linha, &tamanho, arquivo)) != -1) {
            //     printf("Linha lida (%zd caracteres): %s", bytes_lidos, linha);
            // } // colocar tudo numa linha so e ai passar normal

            // printf("teste %s", linha);

            // free(linha); // Libera a memória alocada pela função getline
            // fclose(arquivo);

        }else{
            printf("Muitos argumentos\n");
            return 1;
        }
    }

  return 0;
}

int sequential_style(char *cmd_line, size_t len, ssize_t read){
    pid_t pid;

    char *token; // strtok & recebe strings sem limitacao
    char **args_execvp = (char **)malloc((len + 1) * sizeof(char *)); // aloca de acordo com o tam da cmd line

    if (args_execvp == NULL) {
        perror("Erro na alocacao de memoria\n");
        return 1;
    }

    int flag_semicolon = 0; // muda do sequencial simples para o com ;

    for (int i = 0; cmd_line[i] != '\0'; i++) { // verifica se são varias sequenciais, ou seja, se tem ;
        if (cmd_line[i] == ';') {
            flag_semicolon = 1;
            break;
        }
    }

    if (flag_semicolon == 1) { // COM ;

        int c1 = 0;
        char **args_execvp_semicolon = (char **)malloc((len + 1) * sizeof(char *)); // recebe comandos ate ;

        if (args_execvp_semicolon == NULL) {
            perror("Erro na alocacao de memoria\n");
            return 1;
        }

        token = strtok(cmd_line, ";"); // separa em tokens
        while (token != NULL) {
            args_execvp_semicolon[c1] = token;
            token = strtok(NULL, ";");
            c1++;
        }
        args_execvp_semicolon[c1] = NULL; // coloca null no fim do comando

        // pid_t pid1 = fork(); // cria um processo filho
        
        // if(pid1 > 0){
        //     wait(NULL);
        //     exit(0);
        // }

        for (int i = 0; args_execvp_semicolon[i] != NULL; i++) {
        
            int c2 = 0;
            char *aes = args_execvp_semicolon[i];  // variavel q recebe todo o comando

            token = strtok(aes, " "); // divide o comando separa em tokens
            while (token != NULL) {
                args_execvp[c2] = token;
                token = strtok(NULL, " ");
                c2++;
            }
            args_execvp[c2] = NULL; // coloca null no fim do comando

            pid = fork(); // cria um processo filho
            
            if (pid < 0) {
                fprintf(stderr, "Fork Failed");
                free(args_execvp);
                free(args_execvp_semicolon);
                return 1;
            } else if (pid == 0) { // se for o processo filho, executa
                if(strcmp(args_execvp[0], "exit") == 0){
                    exit(0);
                }
                if(execvp(args_execvp[0], args_execvp) == -1){
                    free(args_execvp);
                    free(args_execvp_semicolon);
                    perror("execvp");
                    exit(0);
                }
            } else { // se nao, eh o pai, fica em wait
                if(strcmp(args_execvp[0], "exit") == 0){
                    exit(0);
                }
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
            free(args_execvp);
            return 1;

        } else if (pid == 0) { // se for o processo filho, executa
            if(strcmp(args_execvp[0], "exit") == 0){
                exit(0);
            }
            if(execvp(args_execvp[0], args_execvp) == -1){
                free(args_execvp);
                perror("execvp");
                exit(0);
            }
        } else { // se nao, eh o pai, fica em wait
            if(strcmp(args_execvp[0], "exit") == 0){
                exit(0);
            }
            wait(NULL);
        }
    }

    free(args_execvp);

    return 0;
}

void *execute_parallel(void *arg){
    char *command = (char*)arg;

    if(strcmp(command, "exit") == 0){
        pthread_exit(NULL);
    }

    system(command);
}

int parallel_style(char *cmd_line, size_t len, ssize_t read){
    
    char *token;
    char **args_par = (char **)malloc((len+1) * sizeof(char *));
    int c = 0;

    token = strtok(cmd_line, ";"); // separa em tokens
    while (token != NULL) {
        args_par[c] = token;
        token = strtok(NULL, ";");
        c++;
    }
    args_par[c] = NULL; // coloca null no fim do comando

    pthread_t threads[c];

    for(int i = 0 ; i < c ; i++){
        
        if(pthread_create(&threads[i], NULL, execute_parallel, (void * )args_par[i]) != 0) {
            perror("Erro ao criar a thread");
            return 1;
        }
    }

    for(int i = 0 ; i < c ; i++){
        pthread_join(threads[i], NULL);
    }

    free(args_par);

}

// n ta reconhecendo troca de styles entre ;
// n ta reconhecendo exit entre comandos ;

// int batch(char *filename){

// }
