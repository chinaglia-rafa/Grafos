#include "logger.h"

struct Grafo {
    int size;
    int type;               // 0 = grafo, 1 = digrafo
    int item[100][100];
    int index[100];
    char filename[200];
    short is_from_file;
};

enum cor {white = 'b', grey = 'c', black = 'p'};
typedef enum cor enum_color;

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
    m.filename[0] = '\0';
    m.is_from_file = 0;
    for (int i = 0; i < size; i++) {
        m.index[i] = i;
        for (int j = 0; j < size; j++)
            m.item[i][j] = 0;
    }

    return m;
}

int indexAt(int pos, struct Grafo m) {
    return m.index[pos];
}

int indexOf(int query, struct Grafo m) {
    for (int i = 0; i < m.size; i++)
        if (m.index[i] == query) return i;

    return -1;
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


    int origin, destiny, peso, indexCounter = 0;
    while (fscanf(f, "%d %d %d", &origin, &destiny, &peso) != EOF) {
        if (DEBUG) printf("Lendo aresta (origem %d, destino %d, peso %d)\n", origin, destiny, peso);
        sprintf(log_text, "Lendo aresta (origem %d, destino %d, peso %d)", origin, destiny, peso);
        log_to_file(log_text);
        int indexOrigin = indexOf(origin, m);
        int indexDestiny = indexOf(destiny, m);
        if (indexOrigin == -1) {
            m.index[indexCounter] = indexOrigin;
            indexOrigin = indexCounter;
            indexCounter++;
        }
        if (indexDestiny == -1) {
            m.index[indexCounter] = indexDestiny;
            indexDestiny = indexCounter;
            indexCounter++;
        }

        m.item[indexOrigin][indexDestiny] = 1;
        if (m.type == 0) m.item[indexDestiny][indexOrigin] = 1;
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
    for (int i = 0; i < m.size; i++) printf("%d ", m.index[i]);
    printf("\n    ");
    for (int i = 0; i < m.size; i++) printf("--");
    printf("\n");
    for (int i = 0; i < m.size; i++) {
        for (int j = 0; j < m.size; j++) {
            if (j == 0) printf("%d | ", m.index[i]);
            printf("%d ", m.item[i][j]);
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
    if (DEBUG) printf("Visitando vértice %c (%d) no tempo %d\n", indexAt(i, m) + 'q', i, dfs_tempo);
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
**  Faz uma busca em largura. Raiz = -1 indica que a raiz é o primeiro elemento.
**/
struct Tabela_largura busca_largura(struct Grafo m, int raiz) {
    char *log_text = (char*)malloc(sizeof(char) * 500);

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

void convert_to_grafo(struct Grafo* m) {
    for (int i = 0; i < m->size; i++) {
        for (int j = 0; j < m->size; j++) {
            if (m->item[i][j] != 0) m->item[j][i] = m->item[i][j];
         }
     }
}
