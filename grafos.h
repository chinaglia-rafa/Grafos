#include "logger.h"
#include <math.h>

struct Grafo {
    int size;
    //  0 = grafo, 1 = digrafo
    int type;
    int item[100][100];
    int index[100];
    char filename[200];
    short is_from_file;
    //  indica se o grafo foi editado e não salvo
    short edited;
};

enum cor {white = 'b', grey = 'c', black = 'p'};
typedef enum cor enum_color;

struct edge {
    int origin;
    int destiny;
    int w;
};

struct static_list {
    struct edge edge;
    int next;
};

struct Linha_kruskal {
    int index;
    int family;
};

struct Tabela_kruskal {
    int size;
    int has_ciclo;
    struct Grafo grafo;
    struct Linha_kruskal linha[99];
};

struct Linha_dijkstra {
    int index;
    int d;
    int parent;
    short done;
    short on_queue;
};

struct Tabela_dijkstra {
    int raiz;
    int size;
    struct Linha_dijkstra linha[100];
    struct Grafo grafo;
};

struct Linha_bellford {
    int index;
    int d;
    int parent;
};

struct Tabela_bellford {
    int raiz;
    int size;
    struct Linha_bellford linha[100];
    struct Grafo grafo;
};

struct Linha_profundidade {
    int index;
    int d;
    int f;
    enum_color color;
};

struct Tabela_profundidade {
    int raiz;
    int size;
    struct Linha_profundidade linha[100];
    struct Grafo grafo;
};

struct Linha_largura {
    int index;
    int parent;
    int d;
    enum_color color;
};

struct Tabela_largura {
    int raiz;
    int size;
    struct Linha_largura linha[100];
    struct Grafo grafo;
};

//  TODO: fazer uma fila digna
struct Queue {
    int item[200];
    int bottom;
    int top;
};

//  TODO: aprender a programar direito para não precisar fazer isso
int dfs_tempo = 0;

/**
**  Retorna uma Grafo inicializada
**/
struct Grafo make_matriz(int size, int type) {
    struct Grafo m;
    m.size = size;
    m.type = type;
    m.edited = 0;
    m.filename[0] = '\0';
    m.is_from_file = 0;
    for (int i = 0; i < size; i++) {
        m.index[i] = -1;
        for (int j = 0; j < size; j++)
            m.item[i][j] = 0;
    }

    return m;
}

int min(int a, int b) {
    if (a < b) return a;
    return b;
}

int indexAt(int pos, struct Grafo m) {
    return m.index[pos];
}

int indexOf(int query, struct Grafo m) {
    for (int i = 0; i < m.size; i++)
        if (m.index[i] == query) return i;

    return -1;
}

short rm_edge(int o, int d, struct Grafo* m) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Removendo aresta (%d, %d)", indexAt(o, *m), indexAt(d, *m));
    log_to_file(log_text);
    free(log_text);

    if (m->item[o][d] == 0) {
        printf("Aresta (%d, %d) não existe.\n", indexAt(o, *m), indexAt(d, *m));
        return 0;
    }
    m->item[o][d] = 0;
    if (m->type == 0) m->item[d][o] = 0;

    return 1;
}

short add_vertex(int vertex, struct Grafo* m) {
    if (m->size == 100) return 0;

    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Criando vértice %d", vertex);
    log_to_file(log_text);
    free(log_text);

    m->index[m->size] = vertex;
    m->size += 1;

    return 1;
}

short add_edge(int o, int d, int p, struct Grafo* m) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Criando aresta (%d, %d) com peso %d", indexAt(o, *m), indexAt(d, *m), p);
    log_to_file(log_text);

    if (m->item[o][d] != 0) {
        sprintf(log_text, "Aresta (%d, %d) já existe com peso %d.\nAtualizando peso para %d.\n", indexAt(o, *m), indexAt(d, *m), m->item[o][d], p);
        log_to_file(log_text);
        free(log_text);
        printf("Aresta (%d, %d) já existe com peso %d.\nAtualizando peso para %d.\n", indexAt(o, *m), indexAt(d, *m), m->item[o][d], p);
        m->item[o][d] = p;
        return 1;
    }
    m->item[o][d] = p;
    if (m->type == 0 && m->item[d][o] != 0) {
        sprintf(log_text, "Aresta (%d, %d) já existe com peso %d.\nAtualizando peso para %d.\n", indexAt(d, *m), indexAt(o, *m), m->item[d][o], p);
        log_to_file(log_text);
        free(log_text);
        printf("Aresta (%d, %d) já existe com peso %d.\nAtualizando peso para %d.\n", indexAt(d, *m), indexAt(o, *m), m->item[d][o], p);
        m->item[d][o] = p;
        return 1;
    }
    if (m->type == 0) m->item[d][o] = p;

    if (m->type == 0)
        printf("Aresta %d -- %d criada com peso %d.\n", indexAt(o, *m), indexAt(d, *m), p);
    else
        printf("Aresta %d -> %d criada com peso %d.\n", indexAt(o, *m), indexAt(d, *m), p);

    return 1;
}

short rm_vertex(int index, struct Grafo* m) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Removendo vértice %d", indexAt(index, *m));
    log_to_file(log_text);
    free(log_text);
    //  Removendo linha e subindo elementos abaixo
    for (int i = index + 1; i < m->size; i++) {
        for (int j = 0; j < m->size; j++) {
            m->item[i - 1][j] = m->item[i][j];
        }
    }
    for (int j = 0; j < m->size; j++) m->item[m->size - 1][j] = 0;
    //  Removendo coluna e arrastando elementos para a esquerda
    for (int i = index + 1; i < m->size; i++) {
        for (int j = 0; j < m->size; j++) {
            m->item[j][i - 1] = m->item[j][i];
        }
    }
    for (int i = 0; i < m->size; i++) m->item[i][m->size - 1] = 0;
    //  Movendo índices
    for (int i = index + 1; i < m->size; i++) m->index[i - 1] = m->index[i];
    m->size -= 1;

    return 1;
}

short adj_to_file(struct Grafo m, char *filename) {
    FILE* f = fopen(filename, "w");
    if (f == NULL) {
        printf("[ERRO]: Arquivo de salvamento de grafo não encontrado!\n");
        return 0;
    }
    fprintf(f, "%d\n", m.type);
    fprintf(f, "%d\n", m.size);
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (m.item[i][j] == 0) continue;
            fprintf(f, "%d %d %d\n", indexAt(i, m), indexAt(j, m), m.item[i][j]);
        }
    }
    fclose(f);

    return 1;
}

struct Grafo load_grafo_from_file(char *filename) {
    FILE* f = fopen(filename, "r");

    int tipo, size;
    fscanf(f, "%d", &tipo);
    fscanf(f, "%d", &size);

    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "====== Carregando (di)grafo \"%s\" ======", filename);
    log_to_file(log_text);


    if (DEBUG) printf("Tipo de grafo encontrado: %d\n", tipo);
    sprintf(log_text, "Tipo de grafo encontrado: %d", tipo);
    log_to_file(log_text);
    if (DEBUG) printf("Número de vértices: %d\n", size);
    sprintf(log_text, "Número de vértices: %d", size);
    log_to_file(log_text);
    if (DEBUG) printf("Iniciando leitura das arestas\n");
    sprintf(log_text, "Iniciando leitura das arestas");
    log_to_file(log_text);

    struct Grafo m = make_matriz(size, tipo);


    int origin, destiny, peso, indexCounter = 0, bigger = 0;
    while (fscanf(f, "%d %d %d", &origin, &destiny, &peso) != EOF) {
        if (DEBUG) printf("Lendo aresta (origem %d, destino %d, peso %d)\n", origin, destiny, peso);
        sprintf(log_text, "Lendo aresta (origem %d, destino %d, peso %d)", origin, destiny, peso);
        log_to_file(log_text);
        int indexOrigin = indexOf(origin, m);
        int indexDestiny = indexOf(destiny, m);
        if (indexOrigin == -1) {
            printf("Novo vértice: %d\n", origin);
            m.index[indexCounter] = origin;
            if (origin > bigger) bigger = origin;
            indexOrigin = indexCounter;
            indexCounter++;
        }
        if (indexDestiny == -1) {
            printf("Novo vértice: %d\n", destiny);
            m.index[indexCounter] = destiny;
            if (destiny > bigger) bigger = destiny;
            indexDestiny = indexCounter;
            indexCounter++;
        }

        m.item[indexOrigin][indexDestiny] = peso;
        if (m.type == 0) m.item[indexDestiny][indexOrigin] = peso;
    }

    //  Verificando vértices sobrando
    if (DEBUG) printf("Agora que as arestas foram consideradas, precisamos ver se existem vértices isolados");
    sprintf(log_text, "Agora que as arestas foram consideradas, precisamos ver se existem vertices isolados");
    log_to_file(log_text);
    for (int i = indexCounter; i < size; i++) {
        //  Começa a criar novos índices seguindo do maior encontrado
        if (DEBUG) printf("\nCriando vértice %d sem arestas.", bigger + i - indexCounter + 1);
        sprintf(log_text, "Criando vertice %d sem arestas.", bigger + i - indexCounter + 1);
        log_to_file(log_text);
        m.index[indexCounter] = bigger + i - indexCounter + 1;
        indexCounter++;
    }

    m.is_from_file = 1;
    strcpy(m.filename, filename);

    fclose(f);

    sprintf(log_text, "====== (di)grafo \"%s\" carregado com sucesso. ======", filename);
    log_to_file(log_text);

    return m;
}

void print_matriz(struct Grafo m) {
    printf("\n    ");
    for (int i = 0; i < m.size; i++) printf(" %d ", m.index[i]);
    printf("\n    ");
    for (int i = 0; i < floor(m.size*2); i++) printf("--");
    printf("\n");
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (j == 0) printf("%2d | ", m.index[i]);
            printf("%2d ", m.item[i][j]);
         }
         printf("\n");
     }
}

void print_matriz_with_char(struct Grafo m, char offset) {
    printf("\n    ");
    for (int i = 0; i < m.size; i++) printf("%c ", m.index[i] + offset);
    printf("\n    ");
    for (int i = 0; i < m.size; i++) printf("--");
    printf("\n");
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (j == 0) printf("%c | ", m.index[i] + offset);
            printf("%d ", m.item[i][j]);
         }
         printf("\n");
     }
}

void tbl_prof_reset(struct Tabela_profundidade* t) {
    t->size = 0;
    t->raiz = 0;
    for (int i = 0; i < 100; i++) {
        t->linha[i].color = white;
        t->linha[i].d = -1;
        t->linha[i].f = -1;
        t->linha[i].index = -1;
    }
}

void tbl_dijkstra_reset(struct Tabela_dijkstra* t) {
    t->size = 0;
    t->raiz = 0;
    for (int i = 0; i < 100; i++) {
        t->linha[i].index = -1;
        t->linha[i].d = -1;
        t->linha[i].parent = -1;
        t->linha[i].done = 0;
        t->linha[i].on_queue = 1;
    }
}

void tbl_bellford_reset(struct Tabela_bellford* t) {
    t->size = 0;
    t->raiz = 0;
    for (int i = 0; i < 100; i++) {
        t->linha[i].index = -1;
        t->linha[i].d = -1;
        t->linha[i].parent = -1;
    }
}

void tbl_larg_reset(struct Tabela_largura* t) {
    t->size = 0;
    t->raiz = 0;
    for (int i = 0; i < 100; i++) {
        t->linha[i].color = white;
        t->linha[i].d = -1;
        t->linha[i].index = -1;
        t->linha[i].parent = -1;
    }
}

void tbl_prof_visit(struct Tabela_profundidade* t, int i, struct Grafo m) {
    t->linha[i].color = grey;
    dfs_tempo += 1;
    t->linha[i].d = dfs_tempo;
    if (DEBUG) printf("Visitando vértice %c (%d) no tempo %d\n", indexAt(i, m) + '0', i, dfs_tempo);
    for (int j = 0; j < m.size; j++) {
        if (m.item[i][j] == 0) continue;
        if (t->linha[j].color == white) {
            tbl_prof_visit(t, j, m);
        }
    }
    t->linha[i].color = black;
    dfs_tempo += 1;
    t->linha[i].f = dfs_tempo;
}

void push(struct Queue* q, int i) {
    q->top++;
    q->item[q->top] = i;
}

int pop(struct Queue* q) {
    if (q->bottom > q->top) return -1;
    int e = q->item[q->bottom];
    q->bottom++;

    return e;
}

/**
**  Encontra o menor valor de caminho numa tabela Dijkstra e retorna ele,
**  ou -1 caso a pilha esteja vazia
**/
int get_dijkstra_min(struct Tabela_dijkstra t) {
    int min = -1;
    //  Encontra o primeiro elemento na pilha para ser referência de menor
    for (int i = 0; i < t.size; i++) {
        if (t.linha[i].on_queue) min = i; break;
    }

    for (int i = 0; i < t.size; i++) {
        if (t.linha[i].d == -1 || !t.linha[i].on_queue) continue;
        if (t.linha[i].d < t.linha[min].d) min = i;
    }

    return min;
}

/**
**  Monta o caminho mínimo usando Bellman-Ford. Raiz = -1 indica que a raiz é o primeiro elemento.
**/
struct Tabela_bellford path_bellford(struct Grafo m, int raiz) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Começando caminho usando Bellman-Ford.");
    log_to_file(log_text);

    struct Tabela_bellford tabela;
    tbl_bellford_reset(&tabela);
    tabela.size = m.size;
    tabela.grafo = m;
    int index_raiz;
    if (raiz == -1) index_raiz = 0;
    else {
        index_raiz = indexOf(raiz, m);
        if (index_raiz == -1) {
            printf("[ERRO]: Raiz não encontrada.");
            sprintf(log_text, "[ERRO]: Raiz nao encontrada.");
            log_to_file(log_text);
            //  TODO: VER ISSO
            struct Tabela_bellford null_struct;
            null_struct.size = -1;
            return null_struct;
        }
    }
    if (raiz == -1) tabela.raiz = 0;
    else tabela.raiz = raiz;

    for (int i = 0; i < tabela.size; i++) {
        tabela.linha[i].index = m.index[i];
        if (i == index_raiz) tabela.linha[i].d = 0;
    }

    for (int c = 0; c < tabela.size - 1; c++) {
        for (int i = 0; i < tabela.size; i++) {
            for (int j = 0; j < tabela.size; j++) {
                if (m.item[i][j] == 0) continue;
                // Relaxando (i, j)
                if (tabela.linha[j].d == -1 || tabela.linha[j].d > tabela.linha[i].d + m.item[i][j]) {
                    if (DEBUG) printf("Relaxando (%d, %d) de %d para %d\n", indexAt(i, m), indexAt(j, m), tabela.linha[j].d, tabela.linha[i].d + m.item[i][j]);
                    sprintf(log_text, "Relaxando (%d, %d) de %d para %d\n", indexAt(i, m), indexAt(j, m), tabela.linha[j].d, tabela.linha[i].d + m.item[i][j]);
                    log_to_file(log_text);
                    tabela.linha[j].d = tabela.linha[i].d + m.item[i][j];
                    tabela.linha[j].parent = i;
                }
            }
        }
    }

    for (int c = 0; c < tabela.size; c++) {
        for (int i = 0; i < tabela.size; i++) {
            for (int j = 0; j < tabela.size; j++) {
                if (m.item[i][j] == 0) continue;
                if (tabela.linha[j].d > tabela.linha[i].d + m.item[i][j]) {
                    printf("[ERRO]: Ciclo negativo encontrado.");
                    sprintf(log_text, "[ERRO]: Ciclo negativo encontrado.");
                    log_to_file(log_text);
                    //  TODO: VER ISSO
                    struct Tabela_bellford null_struct;
                    null_struct.size = -2;
                    return null_struct;
                }
            }
        }
    }

    printf("Caminho mínimo gerado usando Bellman-Ford!\n");
    sprintf(log_text, "Caminho mínimo gerado usando Bellman-Ford!\n");
    log_to_file(log_text);

    return tabela;
}

/**
**  Monta o caminho mínimo usando Dijkstra. Raiz = -1 indica que a raiz é o primeiro elemento.
**/
struct Tabela_dijkstra path_dijkstra(struct Grafo m, int raiz) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    sprintf(log_text, "Começando caminho usando Dijkstra.");
    log_to_file(log_text);

    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (m.item[i][j] < 0) {
                printf("[ERRO]: Aresta com peso negativo encontrada! Impossível usar Dijkstra.");
                sprintf(log_text, "[ERRO]: Aresta com peso negativo encontrada! Impossivel usar Dijkstra.");
                log_to_file(log_text);
                struct Tabela_dijkstra null_struct;
                null_struct.size = -1;
                return null_struct;
            }
        }

    }

    struct Tabela_dijkstra tabela;
    tbl_dijkstra_reset(&tabela);
    tabela.size = m.size;
    tabela.grafo = m;
    int index_raiz;
    if (raiz == -1) index_raiz = 0;
    else {
        index_raiz = indexOf(raiz, m);
        if (index_raiz == -1) {
            printf("[ERRO]: Raiz não encontrada.");
            sprintf(log_text, "[ERRO]: Raiz nao encontrada.");
            log_to_file(log_text);
            //  TODO: VER ISSO
            struct Tabela_dijkstra null_struct;
            null_struct.size = -1;
            return null_struct;
        }
    }
    if (raiz == -1) tabela.raiz = 0;
    else tabela.raiz = raiz;

    for (int i = 0; i < tabela.size; i++) {
        tabela.linha[i].index = m.index[i];
        if (i == index_raiz) tabela.linha[i].d = 0;
    }

    //  Contador responsável por ver quantos elementos ainda estão na pilha
    int queue_count = tabela.size;
    //  Índice atual
    int current_vertex = -1;
    while (queue_count > 0) {
        current_vertex = get_dijkstra_min(tabela);
        if (current_vertex == -1) break;
        if (DEBUG) printf("Menor distância atual = %d (%d)\n", indexAt(current_vertex, m), tabela.linha[current_vertex].d);
        sprintf(log_text, "Menor distância atual = %d (%d)\n", indexAt(current_vertex, m), tabela.linha[current_vertex].d);
        log_to_file(log_text);

        tabela.linha[current_vertex].on_queue = 0;
        //  Relaxa as arestas entre current_vertex e seus adjacentes
        for (int i = 0; i < tabela.size; i++) {
            if (m.item[current_vertex][i] == 0) continue;
            //  Relaxando (current_vertex --> i)
            if (tabela.linha[i].d == -1 || tabela.linha[i].d > tabela.linha[current_vertex].d + m.item[current_vertex][i]) {
                if (DEBUG) printf("Relaxando (%d, %d) de %d para %d\n", indexAt(current_vertex, m), indexAt(i, m), tabela.linha[i].d, tabela.linha[current_vertex].d + m.item[current_vertex][i]);
                sprintf(log_text, "Relaxando (%d, %d) de %d para %d\n", indexAt(current_vertex, m), indexAt(i, m), tabela.linha[i].d, tabela.linha[current_vertex].d + m.item[current_vertex][i]);
                log_to_file(log_text);
                tabela.linha[i].d = tabela.linha[current_vertex].d + m.item[current_vertex][i];
                tabela.linha[i].parent = current_vertex;
            }
        }
    }

    printf("Caminho mínimo gerado usando Dijkstra!\n");
    sprintf(log_text, "Caminho minimo gerado usando Dijkstra!\n");
    log_to_file(log_text);

    return tabela;
}

/**
**  Faz uma busca em largura. Raiz = -1 indica que a raiz é o primeiro elemento.
**/
struct Tabela_largura busca_largura(struct Grafo m, int raiz) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    if (DEBUG) printf("Iniciando busca em largura\n");
    sprintf(log_text, "Iniciando busca em largura");
    log_to_file(log_text);
    struct Tabela_largura tabela;
    tbl_larg_reset(&tabela);
    tabela.size = m.size;
    tabela.grafo = m;
    int index_raiz;
    if (raiz == -1) index_raiz = 0;
    else {
        index_raiz = indexOf(raiz, m);
        if (index_raiz == -1) {
            printf("[ERRO]: Raiz não encontrada.");
            sprintf(log_text, "[ERRO]: Raiz nao encontrada.");
            log_to_file(log_text);
            //  TODO: VER ISSO
            struct Tabela_largura null_struct;
            null_struct.size = -1;
            return null_struct;
        }
    }
    if (raiz == -1) tabela.raiz = 0;
    else tabela.raiz = raiz;

    tabela.linha[index_raiz].color = grey;
    tabela.linha[index_raiz].d = 0;

    struct Queue queue;
    queue.bottom = 0;
    queue.top = -1;

    push(&queue, index_raiz);
    int current_index = pop(&queue);
    if (DEBUG) printf("Raiz do grafo é %d \n", index_raiz);
    while (current_index != -1) {
        if (DEBUG) printf("Buscando irmão branco de %d (indice %d)\n", indexAt(current_index, m), current_index);
        for (int i = 0; i < m.size; i++) {
            if (m.item[current_index][i] == 0) continue;
            if (DEBUG) printf("%d é adjacente a %d\n", indexAt(i, m), indexAt(current_index, m));
            if (tabela.linha[i].color == white) {
                tabela.linha[i].color = grey;
                tabela.linha[i].d = tabela.linha[current_index].d + 1;
                tabela.linha[i].parent = indexAt(current_index, m);
                push(&queue, i);
            }
        }
        tabela.linha[current_index].color = black;
        current_index = pop(&queue);
    }

    return tabela;
}

/**
**  Faz uma busca por profundidade. Raiz = -1 indica que a raiz é o primeiro elemento.
**/
struct Tabela_profundidade busca_profundidade(struct Grafo m, int raiz) {
    char *log_text = (char*)malloc(sizeof(char) * 500);

    dfs_tempo = 0;

    struct Tabela_profundidade tabela;
    tbl_prof_reset(&tabela);
    tabela.size = m.size;
    tabela.grafo = m;
    int index_raiz;
    if (raiz == -1) index_raiz = 0;
    else {
        index_raiz = indexOf(raiz, m);
        if (index_raiz == -1) {
            printf("[ERRO]: Raiz não encontrada.");
            sprintf(log_text, "[ERRO]: Raiz nao encontrada.");
            log_to_file(log_text);
            //  TODO: VER ISSO
            struct Tabela_profundidade null_struct;
            null_struct.size = -1;
            return null_struct;
        }
    }
    if (raiz == -1) tabela.raiz = 0;
    else tabela.raiz = raiz;

    int i = -1;
    short isFirst = 1;
    int current_raiz = 0;
    while (i < m.size - 1) {
        if (isFirst) {
            current_raiz = index_raiz;
            isFirst = 0;
        } else {
            i++;
            current_raiz = i;
        }
        if (tabela.linha[current_raiz].color == white) {
            tbl_prof_visit(&tabela, current_raiz, m);
            printf("Terminei de visitar %c (%d)\n", indexAt(current_raiz, m) + 'q', current_raiz);
        }
    }

    return tabela;
}

void print_tabela_profundidade(struct Tabela_profundidade t) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice | cor |  d  |  f  |\n");
    printf("  |_________|_____|_____|_____|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |   %3d   |  %c  | %3d | %3d |\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].f);
        else
            printf("  |   %3d   |  %c  | %3d | %3d |\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].f);
    }
}

void print_tabela_profundidade_with_char(struct Tabela_profundidade t, char offset) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice | cor |  d  |  f  |\n");
    printf("  |_________|_____|_____|_____|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |    %c    |  %c  | %3d | %3d |\n", t.grafo.index[i] + offset, t.linha[i].color, t.linha[i].d, t.linha[i].f);
        else
            printf("  |    %c    |  %c  | %3d | %3d |\n", t.grafo.index[i] + offset, t.linha[i].color, t.linha[i].d, t.linha[i].f);
    }
}

void print_tabela_largura(struct Tabela_largura t) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice | cor |  d  | pai |\n");
    printf("  |_________|_____|_____|_____|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |   %3d   |  %c  | %3d | %3d |\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].parent);
        else
            printf("  |   %3d   |  %c  | %3d | %3d |\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].parent);
    }
}

void print_tabela_dijkstra(struct Tabela_dijkstra t) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice |  d  |  pai  |\n");
    printf("  |_________|_____|_______|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |   %3d   | %3d | %5d |\n", indexAt(i, t.grafo), t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
        else
            printf("  |   %3d   | %3d | %5d |\n", indexAt(i, t.grafo), t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
    }
}

void print_tabela_bellford(struct Tabela_bellford t) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice |  d  |  pai  |\n");
    printf("  |_________|_____|_______|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |   %3d   | %3d | %5d |\n", indexAt(i, t.grafo), t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
        else
            printf("  |   %3d   | %3d | %5d |\n", indexAt(i, t.grafo), t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
    }
}

void print_tabela_largura_with_char(struct Tabela_largura t, char offset) {
    printf("  Raiz é o elemento %d\n", indexAt(t.raiz, t.grafo));
    printf("   ___________________________\n");
    printf("  | vértice | cor |  d  | pai |\n");
    printf("  |_________|_____|_____|_____|\n");
    for (int i = 0; i < t.size; i++) {
        if (t.raiz == t.grafo.index[i])
            printf("* |    %c    |  %c  | %3d | %3d |\n", t.grafo.index[i] + offset, t.linha[i].color, t.linha[i].d, t.linha[i].parent);
        else
            printf("  |    %c    |  %c  | %3d | %3d |\n", t.grafo.index[i] + offset, t.linha[i].color, t.linha[i].d, t.linha[i].parent);
    }
}

void save_tabela_prof_to_file(struct Tabela_profundidade t, char* filename, char* origin_filename) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    char path[500];
    strcpy(path, "saved/");
    strcat(path, filename);
    if (DEBUG) printf("\nSalvando tabela em \"%s\"", path);
    sprintf(log_text, "Salvando tabela em \"%s\"", path);
    log_to_file(log_text);
    free(log_text);
    FILE* f = fopen(path, "w");
    fprintf(f, "%d\n", t.raiz);
    fprintf(f, "%s\n", origin_filename);
    for (int i = 0; i < t.size; i++) {
        fprintf(f, "%d %c %d %d\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].f);
    }

    fclose(f);
}

void save_tabela_larg_to_file(struct Tabela_largura t, char* filename, char* origin_filename) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    char path[500];
    strcpy(path, "saved/");
    strcat(path, filename);
    if (DEBUG) printf("\nSalvando tabela em \"%s\"", path);
    sprintf(log_text, "Salvando tabela em \"%s\"", path);
    log_to_file(log_text);
    free(log_text);
    FILE* f = fopen(path, "w");
    fprintf(f, "%d\n", t.raiz);
    fprintf(f, "%s\n", origin_filename);
    for (int i = 0; i < t.size; i++) {
        fprintf(f, "%d %c %d %d\n", t.grafo.index[i], t.linha[i].color, t.linha[i].d, t.linha[i].parent);
    }

    fclose(f);
}

void save_tabela_dijkstra_to_file(struct Tabela_dijkstra t, char* filename, char* origin_filename) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    char path[500];
    strcpy(path, "saved/");
    strcat(path, filename);
    if (DEBUG) printf("\nSalvando tabela em \"%s\"", path);
    sprintf(log_text, "Salvando tabela em \"%s\"", path);
    log_to_file(log_text);
    free(log_text);
    FILE* f = fopen(path, "w");
    fprintf(f, "%d\n", t.raiz);
    fprintf(f, "%s\n", origin_filename);
    for (int i = 0; i < t.size; i++) {
        fprintf(f, "%d %d %d\n", t.grafo.index[i], t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
    }

    fclose(f);
}

void save_tabela_bellford_to_file(struct Tabela_bellford t, char* filename, char* origin_filename) {
    char *log_text = (char*)malloc(sizeof(char) * 500);
    char path[500];
    strcpy(path, "saved/");
    strcat(path, filename);
    if (DEBUG) printf("\nSalvando tabela em \"%s\"", path);
    sprintf(log_text, "Salvando tabela em \"%s\"", path);
    log_to_file(log_text);
    free(log_text);
    FILE* f = fopen(path, "w");
    fprintf(f, "%d\n", t.raiz);
    fprintf(f, "%s\n", origin_filename);
    for (int i = 0; i < t.size; i++) {
        fprintf(f, "%d %d %d\n", t.grafo.index[i], t.linha[i].d, indexAt(t.linha[i].parent, t.grafo));
    }

    fclose(f);
}

void convert_to_grafo(struct Grafo* m) {
    for (int i = 0; i < m->size; i++) {
        for (int j = 0; j < m->size; j++) {
            if (m->item[i][j] != 0) m->item[j][i] = m->item[i][j];
         }
     }
}

struct Tabela_kruskal kruskal(struct Grafo m) {
    struct Tabela_kruskal tabela;
    struct static_list arestas[99];
    tabela.has_ciclo = 0;
    int list_root = -1;
    int list_size = 0;
    //  Inicializa a tabela
    for (int i = 0; i < m.size; i++) {
        tabela.linha[i].index = i;
        tabela.linha[i].family = i;
    }
    //  Zera a lista ordenada de arestas
    for (int i = 0; i < m.size * m.size; i++) arestas[i].next = -1;
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (m.item[i][j] == 0) continue;
            struct edge e;
            e.origin = i;
            e.destiny = j;
            e.w = m.item[i][j];
            //  Insere na lista estática
            if (list_root == -1) {
                list_root = 0;
                arestas[list_size].edge = e;
                if (DEBUG) printf("==> Inserindo (%d, %d) com peso %d na raiz\n", indexAt(arestas[list_size].edge.origin, m), indexAt(arestas[list_size].edge.destiny, m), arestas[list_size].edge.w);
                list_size++;
            } else {
                int current = list_root;
                while (arestas[current].edge.w < e.w && arestas[current].next != -1) {
                    current = arestas[current].next;
                }
                if (e.w < arestas[current].edge.w) {
                    arestas[list_size].edge = e;
                    arestas[list_size].next = current;
                    if (current == list_root) list_root = list_size;
                    if (DEBUG) printf("==> Inserindo (%d, %d) com peso %d Antes de (%d, %d) com peso %d\n", indexAt(arestas[list_size].edge.origin, m), indexAt(arestas[list_size].edge.destiny, m), arestas[list_size].edge.w, indexAt(arestas[current].edge.origin, m), indexAt(arestas[current].edge.destiny, m), arestas[current].edge.w);
                    list_size++;
                } else {
                    arestas[list_size].edge = e;
                    arestas[list_size].next = arestas[current].next;
                    arestas[current].next = list_size;
                    if (DEBUG) printf("==> Inserindo (%d, %d) com peso %d DEPOIS de (%d, %d) com peso %d\n", indexAt(arestas[list_size].edge.origin, m), indexAt(arestas[list_size].edge.destiny, m), arestas[list_size].edge.w, indexAt(arestas[current].edge.origin, m), indexAt(arestas[current].edge.destiny, m), arestas[current].edge.w);
                    list_size++;
                }
            }
        }
    }
    int current = list_root;
    while (current != -1) {
        //  Detecta ciclo
        if (DEBUG) printf("Aresta atual: (%d, %d) com peso %d\n", indexAt(arestas[current].edge.origin, m), indexAt(arestas[current].edge.destiny, m), arestas[current].edge.w);
        if (tabela.linha[arestas[current].edge.origin].family == tabela.linha[arestas[current].edge.destiny].family) {
            tabela.has_ciclo = 1;
            if (DEBUG) printf("Ciclo seria formado por (%d, %d) com peso %d\n", indexAt(arestas[current].edge.origin, m), indexAt(arestas[current].edge.destiny, m), arestas[current].edge.w);
            current = arestas[current].next;
            continue;
        }
        int lowest_family = min(tabela.linha[arestas[current].edge.origin].family, tabela.linha[arestas[current].edge.destiny].family);
        // printf("MIN = %d\n", lowest_family);
        for (int c = 0; c < m.size; c++) {
            if (tabela.linha[c].family == tabela.linha[arestas[current].edge.origin].family || tabela.linha[c].family == tabela.linha[arestas[current].edge.destiny].family)
                tabela.linha[c].family = lowest_family;
        }

        current = arestas[current].next;
    }

    return tabela;
}
