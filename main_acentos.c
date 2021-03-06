#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "print_from_file.h"
#include "grafos_acentos.h"
#include "logger.h"

#define COMMAND_SIZE 200

struct Grafo grafos[10];
//  Total de grafos selecionados
int loaded_grafos = 0;
//  Grafo selecionado
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
    //printf("\033[H\033[J");
    system("cls");
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
            printf("%c [%d] %s carregado de \"%s\" (%d vertices)", selected, i + 1, type_grafo(grafos[i].type), grafos[i].filename, grafos[i].size);
            if (grafos[i].edited) printf(" [Alteracoes nao salvas]");
            if (EDIT_MODE && selected_grafo == i) printf(" <--[EDITANDO]");
            printf("\n");
        }

        if (!EDIT_MODE) printf("\nUse o comando select <id> para selecionar outro grafo como ativo.");
    }
    if (DEBUG)
        printf("\n\nStatus do DEBUG: on");
    else
        printf("\n\nStatus do DEBUG: off");

        if (KEEP)
            printf("\n\nMantendo historico de comandos.");

    char *logfile = get_current_logfile();
    printf("\nLog atual e \"%s\"", logfile);
    printf("\n\n");
}

/**
**  Carrega um grafo de um arquivo enviado por parametro
**/
short load_grafo(char *filename) {
    if (filename == NULL) {
        printf("[ERRO]: o nome do arquivo e obrigatorio.");
        return 0;
    }
    char path[200] = "grafos/";
    strcat(path, filename);
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        printf("\n[ERRO]: arquivo /\"%s\" nao encontrado ou inacessivel.", path);
        FILE* f = fopen(filename, "r");
        if (f != NULL) {
            printf("\n[ERRO]: arquivo encontrado no diretorio raiz!\n");
            printf("        mova o arquivo para o diretorio \"/grafos/\".\n");
        }
        return 0;
    }

    grafos[loaded_grafos] = load_grafo_from_file(path);
    loaded_grafos++;

    printf("\n%s carregado com sucesso.", type_grafo(grafos[loaded_grafos - 1].type));
}

/**
**  Exibe um arquivo enviado por parametro
**/
short cat_file(char *filename) {
    if (filename == NULL) {
        printf("[ERRO]: o nome do arquivo e obrigatorio.");
        return 0;
    }

    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        printf("\n[ERRO]: arquivo /\"%s\" nao encontrado ou inacessivel.", filename);
        FILE* f = fopen(filename, "r");
        if (f != NULL) {
            printf("\n[ERRO]: arquivo encontrado no diretorio raiz!\n");
            printf("        mova o arquivo para o diretorio \"/grafos/\".\n");
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
        printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", opt_as_num);
        return 0;
    }
    selected_grafo = opt_as_num - 1;

    printf("\n* [%d] %s \"%s\" (%d vertices) selecionado.\n", opt_as_num, type_grafo(grafos[selected_grafo].type), grafos[selected_grafo].filename, grafos[selected_grafo].size);

    return 1;
}

int main () {


    char command[COMMAND_SIZE];
    char *token;

    //  MAIN COMMAND LOOP
    while (1) {
        if (!KEEP) clear();
        printf("Bem vindo ao Euler1736 %s.\nUse help ou h para ter ajuda.\n", VERSION);
        if (EDIT_MODE) {
            printf("Voce esta no <MODO DE EDICAO>. Para sair, use o comando [q]uit.\n");
            printf("    Editando [%d] %s carregado de \"%s\" (%d vertices)\n\n", selected_grafo + 1, type_grafo(grafos[selected_grafo].type), grafos[selected_grafo].filename, grafos[selected_grafo].size);
        }
        printf("\n/> ");
        fgets(command, COMMAND_SIZE, stdin);

        token = strtok(command, SEPARATOR);
        purgeBreakLine(token);
        //  ============ MODO DE EDICAO ============
        if (EDIT_MODE) {
            if (strcmp(token, "quit") == 0 || strcmp(token, "q") == 0) {    // EDIT - QUIT
                char *opt;
                opt = strtok(NULL, SEPARATOR);
                purgeBreakLine(opt);
                if (opt != NULL && (strcmp(opt, "--save") == 0 || strcmp(opt, "-s") == 0)) {
                    adj_to_file(grafos[selected_grafo], grafos[selected_grafo].filename);
                }
                EDIT_MODE = 0;
            } else if (strcmp(token, "status") == 0 || strcmp(token, "s") == 0) {    //  EDIT - STATUS
                show_status();
                pause();
            } else if (strcmp(token, "help") == 0 || strcmp(token, "h") == 0) {     //  EDIT - HELP
                print_from_file("help-edit", LANG);
                pause();
            } else if (strcmp(token, "exit") == 0) {         //  EDIT - EXIT (invalido)
                printf("Voce esta no Modo de Edicao. Use o comando >> [q]uit <<");
                pause();
            } else if (strcmp(token, "saveas") == 0) {         //  EDIT - SAVE AS
                char *opt;
                opt = strtok(NULL, SEPARATOR);
                purgeBreakLine(opt);
                if (opt == NULL) {
                    printf("[ERRO]: O comando saveas espera um nome de arquivo <filename>.");
                    pause();
                    continue;
                }
                char path[400] = "grafos/";
                strcat(path, opt);
                adj_to_file(grafos[selected_grafo], path);
                grafos[selected_grafo].edited = 0;
                printf("Arquivo %s salvo com sucesso!\n", path);
                pause();
            } else if (strcmp(token, "rm") == 0) {         //  EDIT - RM
                char *opt;
                opt = strtok(NULL, SEPARATOR);
                purgeBreakLine(opt);
                if (opt == NULL) {
                    printf("[ERRO]: O comando rm espera as opcoes -v, --vertex, -e ou --edge");
                    pause();
                    continue;
                }
                //  Se a opcao escolhida for para remover vertice
                if (strcmp(opt, "-v") == 0 || strcmp(opt, "--vertex") == 0) {
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice informado para remocao.");
                        pause();
                        continue;
                    }
                    int vertex = atoi(opt);
                    int index = indexOf(vertex, grafos[selected_grafo]);
                    if (index == -1) {
                        printf("[ERRO]: Vertice %d nao encontrado.", vertex);
                        pause();
                        continue;
                    }
                    grafos[selected_grafo].edited = 1;
                    rm_vertex(index, &grafos[selected_grafo]);
                    printf("Vertice %d removido.\n", vertex);

                }
                //  Se a opcao escolhida for para remover aresta
                else if (strcmp(opt, "-e") == 0 || strcmp(opt, "--edge") == 0) {
                    //  Pegando vertice de origem
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice de origem informado para remocao.");
                        pause();
                        continue;
                    }
                    int vertex_origin = atoi(opt);
                    int index_origin = indexOf(vertex_origin, grafos[selected_grafo]);
                    if (index_origin == -1) {
                        printf("[ERRO]: Vertice de origem %d nao encontrado.", vertex_origin);
                        pause();
                        continue;
                    }
                    //  Pegando vertice de destino
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice de destino informado para remocao.");
                        pause();
                        continue;
                    }
                    int vertex_destiny = atoi(opt);
                    int index_destiny = indexOf(vertex_destiny, grafos[selected_grafo]);
                    if (index_destiny == -1) {
                        printf("[ERRO]: Vertice de destino %d nao encontrado.", vertex_destiny);
                        pause();
                        continue;
                    }
                    if (!rm_edge(index_origin, index_destiny, &grafos[selected_grafo])) {
                        pause();
                        continue;
                    }
                    grafos[selected_grafo].edited = 1;

                    if (grafos[selected_grafo].type == 0)
                        printf("Aresta %d -- %d removida.\n", vertex_origin, vertex_destiny);
                    else
                        printf("Aresta %d -> %d removida.\n", vertex_origin, vertex_destiny);
                }
                pause();
            } else if (strcmp(token, "add") == 0) {         //  EDIT - ADD
                char *opt;
                opt = strtok(NULL, SEPARATOR);
                purgeBreakLine(opt);
                if (opt == NULL) {
                    printf("[ERRO]: O comando add espera as opcoes -v, --vertex, -e ou --edge");
                    pause();
                    continue;
                }
                //  Se a opcao escolhida for para adicionar vertice
                if (strcmp(opt, "-v") == 0 || strcmp(opt, "--vertex") == 0) {
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice informado para criacao.");
                        pause();
                        continue;
                    }
                    int vertex = atoi(opt);
                    grafos[selected_grafo].edited = 1;
                    add_vertex(vertex, &grafos[selected_grafo]);
                    printf("Vertice %d adicionado.\n", vertex);

                }
                //  Se a opcao escolhida for para adicionar aresta
                else if (strcmp(opt, "-e") == 0 || strcmp(opt, "--edge") == 0) {
                    //  Pegando vertice de origem
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice de origem informado para adicao.");
                        pause();
                        continue;
                    }
                    int vertex_origin = atoi(opt);
                    int index_origin = indexOf(vertex_origin, grafos[selected_grafo]);
                    if (index_origin == -1) {
                        printf("[ERRO]: Vertice de origem %d nao encontrado.", vertex_origin);
                        pause();
                        continue;
                    }
                    //  Pegando vertice de destino
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    if (opt == NULL) {
                        printf("[ERRO]: nenhum vertice de destino informado para adicao.");
                        pause();
                        continue;
                    }
                    int vertex_destiny = atoi(opt);
                    int index_destiny = indexOf(vertex_destiny, grafos[selected_grafo]);
                    if (index_destiny == -1) {
                        printf("[ERRO]: Vertice de destino %d nao encontrado.", vertex_destiny);
                        pause();
                        continue;
                    }
                    opt = strtok(NULL, SEPARATOR);
                    purgeBreakLine(opt);
                    int peso = 1;
                    if (opt != NULL) peso = atoi(opt);

                    if (!add_edge(index_origin, index_destiny, peso, &grafos[selected_grafo])) {
                        pause();
                        continue;
                    }
                    grafos[selected_grafo].edited = 1;
                }
                pause();
            } else if (strcmp(token, "type") == 0) {         //  EDIT - TYPE
                char *opt;
                opt = strtok(NULL, SEPARATOR);
                if (opt == NULL) {
                    printf("tipo de grafo = %s", type_grafo(grafos[selected_grafo].type));
                } else {
                    purgeBreakLine(opt);
                    if (strcmp(opt, "digrafo") == 0) {
                        grafos[selected_grafo].type = 1;
                    } else if (strcmp(opt, "grafo") == 0) {
                        grafos[selected_grafo].type = 0;
                        convert_to_grafo(grafos + selected_grafo);
                    }
                    else {
                        printf("\n[ERRO]: Tipo deve ser \"digrafo\" ou \"grafo\".");
                        pause();
                        continue;
                    }
                    char *log_text = (char*)malloc(sizeof(char) * 500);
                    sprintf(log_text, "<MODO EDICAO> - Tipo de grafo de [%d] alterado para \"%s\"", selected_grafo + 1, opt);
                    log_to_file(log_text);
                    free(log_text);
                    printf("Tipo de grafo alterado para \"%s\"", opt);
                }
                pause();
            } else if (strcmp(token, "adj") == 0) {         // EDIT - ADJ
                printf("Gerando matriz de adjacencia\n%s \"%s\" (%d vertices). (ID = %d)\n", type_grafo(grafos[selected_grafo].type), grafos[selected_grafo].filename, grafos[selected_grafo].size, selected_grafo + 1);
                print_matriz(grafos[selected_grafo]);
                pause();
            }

            continue;
        }

        if (strcmp(token, "help") == 0 || strcmp(token, "h") == 0) {                  //  HELP
            print_from_file("help", LANG);
            pause();
        } else if (strcmp(token, "exit") == 0) {           //  EXIT
            return 1;
        } else if (strcmp(token, "edit") == 0) {           //  EDIT
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
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
                pause();
                continue;
            }
            if (id != selected_grafo) printf("Entrando no [Modo de Edicao] do %s com ID = %d.\nO %s foi selecionado como principal.\n", type_grafo(grafos[id].type), id + 1, type_grafo(grafos[id].type));
            else printf("Entrando no [Modo de Edicao] do %s com ID = %d.", type_grafo(grafos[id].type), id + 1);
            selected_grafo = id;
            pause();
            EDIT_MODE = 1;
        } else if (strcmp(token, "status") == 0 || strcmp(token, "s") == 0) {         //  STATUS
            show_status();
            pause();
        } else if (strcmp(token, "select") == 0) {         //  SELECT
            char* opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            select_grafo(opt);
            char *log_text = (char*)malloc(sizeof(char) * 500);
            sprintf(log_text, "%s [%d] selecionado como ativo.", type_grafo(grafos[selected_grafo].type), selected_grafo + 1);
            log_to_file(log_text);
            free(log_text);
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
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
                pause();
                continue;
            }
            printf("Gerando matriz de adjacencia\n%s \"%s\" (%d vertices). (ID = %d)\n", type_grafo(grafos[id].type), grafos[id].filename, grafos[id].size, id + 1);
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
        } else if (strcmp(token, "keep") == 0) {         //  KEEP
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            if (opt == NULL) {
                printf("keep = ");
                if (KEEP == 1) printf("on");
                else printf("off");
            } else {
                purgeBreakLine(opt);
                if (strcmp(opt, "on") == 0) KEEP = 1;
                else if (strcmp(opt, "off") == 0) KEEP = 0;
                else {
                    printf("\n[ERRO]: Keep deve ser \"on\" ou \"off\".");
                    pause();
                    continue;
                }
                printf("Modo Keep alterado para \"%s\"", opt);
            }
            pause();
        } else if (strcmp(token, "logfile") == 0) {         //  LOGFILE
            char *logfile = get_current_logfile();
            printf("Arquivo de log atual e \"%s\"", logfile);
            pause();
        } else if (strcmp(token, "dfs") == 0) {         //  DFS
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            if (opt == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Raiz nao informanda.");
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
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
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
        } else if (strcmp(token, "bfs") == 0) {         //  BFS
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            if (opt == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Raiz nao informanda.");
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
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
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
                print_tabela_largura(t);
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
        } else if (strcmp(token, "path") == 0) {         //  CAMINHOS - PATH
            char *raiz;
            raiz = strtok(NULL, SEPARATOR);
            purgeBreakLine(raiz);
            if (raiz == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Raiz nao informanda.");
                sprintf(log_text, "[ERRO]: Raiz nao informanda.");
                log_to_file(log_text);
                free(log_text);
                continue;
            }
            char *type;
            type = strtok(NULL, SEPARATOR);
            purgeBreakLine(type);
            if (type == NULL) {
                char *log_text = (char*)malloc(sizeof(char) * 500);
                printf("\n[ERRO]: Tipo de caminho nao informado! Deve ser [d]ijkstra ou [b]ellford.");
                sprintf(log_text, "[ERRO]: Tipo de caminho nao informado! Deve ser [d]ijkstra ou [b]ellford.");
                log_to_file(log_text);
                free(log_text);
                continue;
            }
            if (!(strcmp(type, "d") == 0 || strcmp(type, "dijkstra") == 0 || strcmp(type, "b") == 0 || strcmp(type, "bellford") == 0)) {
                 char *log_text = (char*)malloc(sizeof(char) * 500);
                 printf("\n[ERRO]: Tipo de caminho deve ser [d]ijkstra ou [b]ellford.");
                 sprintf(log_text, "[ERRO]: Tipo de caminho deve ser [d]ijkstra ou [b]ellford.");
                 log_to_file(log_text);
                 free(log_text);
                 continue;
            }
            char *opt;
            opt = strtok(NULL, SEPARATOR);
            purgeBreakLine(opt);
            int id = -1;
            if (opt != NULL && strcmp(opt, "--save") != 0) {
                purgeBreakLine(opt);
                id = atoi(opt) - 1;
            } else {
                id = selected_grafo;
            }
            if (id < 0 || id >= loaded_grafos) {
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
                pause();
                continue;
            }

            if (strcmp(type, "d") == 0 || strcmp(type, "dijkstra") == 0) {  //  PATH - DIJKSTRA
                struct Tabela_dijkstra t = path_dijkstra(grafos[id], atoi(raiz));
                if (t.size == -1) {
                    char *log_text = (char*)malloc(sizeof(char) * 500);
                    printf("\n[ERRO]: Algo saiu errado ao fazer o caminho por Dijkstra.");
                    sprintf(log_text, "[ERRO]: Algo saiu errado ao fazer o caminho por Dijkstra.");
                    log_to_file(log_text);
                    free(log_text);
                } else {
                    print_tabela_dijkstra(t);
                    char filename[200];
                    short willSave = 0;
                    if (opt != NULL && strcmp(opt, "--save") == 0) {
                        willSave = 1;
                        opt = strtok(NULL, SEPARATOR);
                        purgeBreakLine(opt);
                        if (opt != NULL) {
                            strcpy(filename, opt);
                        } else {
                            strcpy(filename, "dijkstra.tbl");
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
                                strcpy(filename, "dijkstra.tbl");
                        } else {
                            strcpy(filename, "dijkstra.tbl");
                        }
                    }
                    if (willSave) {
                        save_tabela_dijkstra_to_file(t, filename, grafos[id].filename);
                    }
                }
            } else if (strcmp(type, "b") == 0 || strcmp(type, "bellford") == 0) {  //  PATH - BELLMAN-FORD
                struct Tabela_bellford t = path_bellford(grafos[id], atoi(raiz));
                if (t.size == -1) {
                    char *log_text = (char*)malloc(sizeof(char) * 500);
                    printf("\n[ERRO]: Algo saiu errado ao fazer o caminho por Bellman-Ford.");
                    sprintf(log_text, "[ERRO]: Algo saiu errado ao fazer o caminho por Bellman-Ford.");
                    log_to_file(log_text);
                    free(log_text);
                } else if (t.size == -2) {
                    char *log_text = (char*)malloc(sizeof(char) * 500);
                    printf("\n[ERRO]: Bellman-Ford retornou FALSO.");
                    sprintf(log_text, "[ERRO]: Bellman-Ford retornou FALSO.");
                    log_to_file(log_text);
                    free(log_text);
                } else {
                    print_tabela_bellford(t);
                    char filename[200];
                    short willSave = 0;
                    if (opt != NULL && strcmp(opt, "--save") == 0) {
                        willSave = 1;
                        opt = strtok(NULL, SEPARATOR);
                        purgeBreakLine(opt);
                        if (opt != NULL) {
                            strcpy(filename, opt);
                        } else {
                            strcpy(filename, "bellford.tbl");
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
                                strcpy(filename, "bellford.tbl");
                        } else {
                            strcpy(filename, "bellford.tbl");
                        }
                    }
                    if (willSave) {
                        save_tabela_bellford_to_file(t, filename, grafos[id].filename);
                    }
                }
            }

            pause();
        } else if (strcmp(token, "props") == 0) {         //  PROPS
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
                printf("\n[ERRO]: <id> \"%d\" de grafo invalido.", id + 1);
                pause();
                continue;
            }

            struct Tabela_largura t = busca_largura(grafos[id], -1);
            short is_conexo = 1;
            //  Verifica se o grafo e conexo
            for (int i = 0; i < t.size; i++) {
                if (t.linha[i].color != black) {
                    is_conexo = 0;
                    break;
                }
            }
            //  Verifica se o grafo e ciclico
            struct Tabela_kruskal k = kruskal(grafos[id]);
            printf("\n==== Propriedades de <%d> - \"%s\" =====\n", id + 1, grafos[id].filename);
            printf("    - Tipo = %s\n", type_grafo(grafos[id].type));
            if (is_conexo) printf("    - (Di)grafo conexo = Sim\n");
                else printf("    - (Di)grafo conexo = Nao\n");

            if (k.has_ciclo) printf("    - (Di)grafo ciclico = Sim\n");
                else printf("    - (Di)grafo ciclico = Nao\n");

            if (!k.has_ciclo && is_conexo) printf("    - (Di)grafo e arvore = Sim\n");
                else if (!k.has_ciclo && !is_conexo) printf("    - (Di)grafo e floresta = Sim\n");
                else printf("    - (Di)grafo e arvore = Nao\n");

            pause();
        } else {
            printf("Comando \"%s\" nao reconhecido.", token);
            pause();
        }
    }


    printf("\n\n");
}
