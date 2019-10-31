#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "print_from_file.h"
#include "grafos.h"
#include "logger.h"

#define COMMAND_SIZE 200

struct Grafo grafos[10];
int loaded_grafos = 0;
int selected_grafo = 0;

/**
**  Remove o \n de uma string
**/
char* purgeBreakLine(char *str) {
    if (str == NULL) return NULL;
    int last_char = strlen(str) - 1;
    if (*(str + last_char) == '\n') {
        *(str + last_char) = '\0';
    }

    return str;
}

/**
**  Limpa a tela magicamente.
**/
void clear() {
    printf("\033[H\033[J");
}

/**
**  Pausa a tela para leitura.
**/
void pause() {
    printf("\nPressione [Enter] para continuar...");
    getchar();
}

char *type_grafo(int type) {
    if (type == 0) return "grafo";
    if (type == 1) return "digrafo";
}

void show_status() {

    if (loaded_grafos == 0) {
        printf("Nenhum grafo carregado.");
    } else {
        printf("Grafos carregados:\n");
        char selected = ' ';
        for (int i = 0; i < loaded_grafos; i++) {
            if (selected_grafo == i)
                selected = '*';
            else
                selected = ' ';
            printf("%c [%d] %s carregado de \"%s\" (%d vértices)\n", selected, i + 1, type_grafo(grafos[i].type), grafos[i].filename, grafos[i].size);
        }

        printf("\nUse o comando select <id> para selecionar outro grafo como ativo.");
    }
    if (DEBUG)
        printf("\n\nStatus do DEBUG: on");
    else
        printf("\n\nStatus do DEBUG: off");

    char *logfile = get_current_logfile();
    printf("\nLog atual é \"%s\"", logfile);
    printf("\n\n");
}

/**
**  Carrega um grafo de um arquivo enviado por parametro
**/
short load_grafo(char *filename) {
    if (filename == NULL) {
        printf("[ERRO]: o nome do arquivo é obrigatório.");
        return 0;
    }
    char path[200] = "grafos/";
    strcat(path, filename);
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("\n[ERRO]: arquivo /\"%s\" não encontrado ou inacessível.", path);
        FILE* f = fopen(filename, "r");
        if (f != NULL) {
            printf("\n[ERRO]: arquivo encontrado no diretório raiz!\n");
            printf("        mova o arquivo para o diretório \"/grafos/\".\n");
        }
        return 0;
    }

    grafos[loaded_grafos] = load_grafo_from_file(path);
    loaded_grafos++;

    printf("\n%s carregado com sucesso.", type_grafo(grafos[loaded_grafos - 1].type));
}

/**
**  Carrega um grafo de um arquivo enviado por parametro
**/
short cat_file(char *filename) {
    if (filename == NULL) {
        printf("[ERRO]: o nome do arquivo é obrigatório.");
        return 0;
    }
    char path[200] = "grafos/";
    strcat(path, filename);
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("\n[ERRO]: arquivo /\"%s\" não encontrado ou inacessível.", path);
        FILE* f = fopen(filename, "r");
        if (f != NULL) {
            printf("\n[ERRO]: arquivo encontrado no diretório raiz!\n");
            printf("        mova o arquivo para o diretório \"/grafos/\".\n");
        }
        return 0;
    }

    char line[500];
    printf("\n");
    while (fgets(line, 500, f)) {
        if (feof(f)) break;
        printf("%s", line);
    }
}

/**
**  Seleciona um grafo de uma lista de grafos carregados
**/
short select_grafo(char *opt) {
    int opt_as_num = atoi(opt);
    if (opt == NULL || opt_as_num < 1 || opt_as_num > loaded_grafos) {
        printf("\n[ERRO]: <id> \"%d\" de grafo inválido.", opt_as_num);
        return 0;
    }
    selected_grafo = opt_as_num - 1;

    printf("\n* [%d] %s \"%s\" (%d vértices) selecionado.\n", opt_as_num, type_grafo(grafos[selected_grafo].type), grafos[selected_grafo].filename, grafos[selected_grafo].size);

    return 1;
}

int main () {
    char command[COMMAND_SIZE];
    char *token;

    //  MAIN COMMAND LOOP
    while (1) {
        // clear();
        printf("Bem vindo ao Euler1736 %s.\nUse help ou h para ter ajuda.\n", VERSION);
        printf("\n/> ");
        fgets(command, COMMAND_SIZE, stdin);

        token = strtok(command, SEPARATOR);
        purgeBreakLine(token);
        if (strcmp(token, "help") == 0 || strcmp(token, "h") == 0) {                  //  HELP
            print_from_file("help", LANG);
            pause();
        } else if (strcmp(token, "exit") == 0) {           //  EXIT
            return 1;
        } else if (strcmp(token, "status") == 0 || strcmp(token, "s") == 0) {         //  STATUS
            show_status();
            pause();
        } else if (strcmp(token, "select") == 0) {         //  SELECT
            char* opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            select_grafo(opt);
            pause();
        } else if (strcmp(token, "cat") == 0) {         //  CAT
            char* filename;
            filename = strtok(NULL, SEPARATOR);
            purgeBreakLine(filename);
            cat_file(filename);
            pause();
        } else if (strcmp(token, "load") == 0 || strcmp(token, "l") == 0) {         //  LOAD
            char *filename;
            filename = strtok(NULL, SEPARATOR);
            purgeBreakLine(filename);
            load_grafo(filename);
            pause();
        } else if (strcmp(token, "adj") == 0) {         //  ADJ
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            int id = 0;
            if (opt != NULL) {
                purgeBreakLine(opt);
                id = atoi(opt) - 1;
            } else {
                id = selected_grafo;
            }
            if (id < 0 || id >= loaded_grafos) {
                printf("\n[ERRO]: <id> \"%d\" de grafo inválido.", id + 1);
                pause();
                continue;
            }
            printf("Gerando matriz de adjacência\n%s \"%s\" (%d vértices). (ID = %d)\n", type_grafo(grafos[id].type), grafos[id].filename, grafos[id].size, id + 1);
            print_matriz(grafos[id]);
            pause();
        } else if (strcmp(token, "debug") == 0) {         //  DEBUG
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            if (opt == NULL) {
                printf("debug = ");
                if (DEBUG == 1) printf("on");
                else printf("off");
            } else {
                purgeBreakLine(opt);
                if (strcmp(opt, "on") == 0) DEBUG = 1;
                else if (strcmp(opt, "off") == 0) DEBUG = 0;
                else {
                    printf("\n[ERRO]: Debug deve ser \"on\" ou \"off\".");
                    pause();
                    continue;
                }
                printf("Modo debug alterado para \"%s\"", opt);
            }
            pause();
        } else if (strcmp(token, "logfile") == 0) {         //  LOGFILE
            char *logfile = get_current_logfile();
            printf("Arquivo de log atual é \"%s\"", logfile);
            pause();
        } else if (strcmp(token, "dfs") == 0) {         //  DFS
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            if (opt == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Raiz não informanda.");
                sprintf(log_text, "[ERRO]: Raiz nao informanda.");
                log_to_file(log_text);
                free(log_text);
                continue;
            }
            int raiz = atoi(opt);
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            int id = 0;
            if (opt != NULL && strcmp(opt, "--save") != 0) {
                purgeBreakLine(opt);
                id = atoi(opt) - 1;
            } else {
                id = selected_grafo;
            }
            if (id < 0 || id >= loaded_grafos) {
                printf("\n[ERRO]: <id> \"%d\" de grafo inválido.", id + 1);
                pause();
                continue;
            }
            struct Tabela_profundidade t = busca_profundidade(grafos[id], raiz);
            if (t.size == -1) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Algo saiu errado ao fazer a busca por profundidade.");
                sprintf(log_text, "[ERRO]: Algo saiu errado ao fazer a busca por profundidade.");
                log_to_file(log_text);
                free(log_text);
            } else {
                print_tabela_profundidade(t);
                char filename[200];
                short willSave = 0;
                if (opt != NULL && strcmp(opt, "--save") == 0) {
                    willSave = 1;
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt != NULL) {
                        strcpy(filename, opt);
                    } else {
                        strcpy(filename, "dfs.tbl");
                    }
                } else {
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt != NULL && strcmp(opt, "--save") == 0) {
                        willSave = 1;
                        opt = strtok(NULL, SEPARATOR);
                        purgeBreakLine(opt);
                        if (opt != NULL)
                            strcpy(filename, opt);
                        else
                            strcpy(filename, "dfs.tbl");
                    } else {
                        strcpy(filename, "dfs.tbl");
                    }
                }
                if (willSave) {
                    save_tabela_prof_to_file(t, filename, grafos[id].filename);
                }

            }
            pause();
        }else if (strcmp(token, "bfs") == 0) {         //  BFS
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            if (opt == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Raiz não informanda.");
                sprintf(log_text, "[ERRO]: Raiz nao informanda.");
                log_to_file(log_text);
                free(log_text);
                continue;
            }
            int raiz = atoi(opt);
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            int id = 0;
            if (opt != NULL && strcmp(opt, "--save") != 0) {
                purgeBreakLine(opt);
                id = atoi(opt) - 1;
            } else {
                id = selected_grafo;
            }
            if (id < 0 || id >= loaded_grafos) {
                printf("\n[ERRO]: <id> \"%d\" de grafo inválido.", id + 1);
                pause();
                continue;
            }
            struct Tabela_largura t = busca_largura(grafos[id], raiz);
            if (t.size == -1) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Algo saiu errado ao fazer a busca em largura.");
                sprintf(log_text, "[ERRO]: Algo saiu errado ao fazer a busca em largura.");
                log_to_file(log_text);
                free(log_text);
            } else {
                print_tabela_largura_with_char(t, 'q');
                char filename[200];
                short willSave = 0;
                if (opt != NULL && strcmp(opt, "--save") == 0) {
                    willSave = 1;
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt != NULL) {
                        strcpy(filename, opt);
                    } else {
                        strcpy(filename, "bfs.tbl");
                    }
                } else {
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt != NULL && strcmp(opt, "--save") == 0) {
                        willSave = 1;
                        opt = strtok(NULL, SEPARATOR);
                        purgeBreakLine(opt);
                        if (opt != NULL)
                            strcpy(filename, opt);
                        else
                            strcpy(filename, "bfs.tbl");
                    } else {
                        strcpy(filename, "bfs.tbl");
                    }
                }
                if (willSave) {
                    save_tabela_larg_to_file(t, filename, grafos[id].filename);
                }

            }
            pause();
        } else {
            printf("Comando \"%s\" não reconhecido.", token);
            pause();
        }
    }


    printf("\n\n");
}
