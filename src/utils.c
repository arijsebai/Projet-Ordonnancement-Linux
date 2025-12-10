#include <stdio.h>

/* Vide le buffer stdin pour éviter les lectures résiduelles */
void clear_buffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
        /* discard */
    }
}

/* Affiche un fichier de configuration pour aider au debug */
void display_config_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Impossible d'ouvrir le fichier de configuration");
        return;
    }

    printf("\n=== Contenu du fichier de configuration (%s) ===\n", filename);

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    printf("============================================\n\n");

    fclose(f);
}
