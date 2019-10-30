#include <stdio.h>
#include <stdlib.h>

struct MatrizAdj {
    int size;
    int type;       // 0 = grafo, 1 = digrafo
    int item[100][100];
    int index[100];
};

int debug = 1;

/**
**  Retorna uma MatrizAdj inicializada
**/
struct MatrizAdj make_matriz(int size, int type) {
    struct MatrizAdj m;
    m.size = size;
    m.type = type;
    for (int i = 0; i < size; i++) {
        m.index[i] = i;
        for (int j = 0; j < size; j++)
            m.item[i][j] = 0;
    }

    return m;
}

int indexAt(int pos, struct MatrizAdj m) {
    return m.index[pos];
}

int indexOf(int query, struct MatrizAdj m) {
    for (int i = 0; i < m.size; i++)
        if (m.index[i] == query) return i;

    return -1;
}

struct MatrizAdj loadGrafoFromFile(char filename[100]) {
    FILE* f = fopen(filename, "r");

    char line[200];
    int tipo, size;
    fgets(line, 200, f);
    fscanf(f, "%d", &tipo);
    fscanf(f, "%d", &size);

    if (debug) printf("Tipo de grafo encontrado: %d\n", tipo);
    if (debug) printf("Número de vérticas: %d\n", size);
    if (debug) printf("Iniciando leitura das arestas\n");

    struct MatrizAdj m = make_matriz(size, tipo);


    int origin, destiny, peso, indexCounter = 0;
    while (fscanf(f, "%d %d %d", &origin, &destiny, &peso) != EOF) {
        printf("Lendo aresta (origem %d, destino %d, peso %d)\n", origin, destiny, peso);
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

    fclose(f);

    return m;
}

void print_matriz(struct MatrizAdj m) {
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

void main () {

    struct MatrizAdj m = loadGrafoFromFile("grafo-1.txt");

    print_matriz(m);

}
