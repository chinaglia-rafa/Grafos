#include <stdio.h>
#include <string.h>

/**
**  A função pega o conteúdo de um arquivo em uma linguagem lang e imprime na tela
**/
short print_from_file(char *text, char *lang) {
    char filename[200];
    strcpy(filename, "texts/");
    strcat(filename, text);
    strcat(filename, ".");
    strcat(filename, lang);

    FILE* f = fopen(filename, "r");
    if (f == NULL) return 0;

    char line[500];
    printf("\n");
    while (fgets(line, 500, f)) {
        if (feof(f)) break;
        printf("%s", line);
    }
}
