#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "../include/process.h" /* doit définir struct process et NAME_LEN */

#ifndef LINE_SIZE
#define LINE_SIZE 256
#endif

/* comparateur : tri par arrival croissant */
static int cmp_arrival(const void *a, const void *b) {
    const struct process *pa = a;
    const struct process *pb = b;
    return (pa->arrival_time - pb->arrival_time);
}

/* Trie le tableau procs[0..n-1] par arrival time */
void sort_by_arrival(struct process *procs, int n) {
    if (!procs || n <= 1) return;
    qsort(procs, n, sizeof(struct process), cmp_arrival);
}

/* parse_config : implementation  */
int parse_config_file(const char *path, struct process **out, int *out_n) {
    if (!path || !out || !out_n) return -1;

    FILE *f = fopen(path, "r");
    if (!f) return -1;

    size_t cap = 16;
    size_t n = 0;
    struct process *arr = malloc(cap * sizeof *arr);
    if (!arr) { fclose(f); return -1; }

    char line[LINE_SIZE];
    int lineno = 0;
    while (fgets(line, sizeof line, f) != NULL) {
        lineno++;

        /* supprimer le '\n' final si présent */
        size_t L = strlen(line);
        if (L > 0 && line[L-1] == '\n') line[L-1] = '\0';

        /* pointer sur premier caractère non blanc */
        char *p = line;
        while (*p && isspace((unsigned char)*p)) p++;

        /* ignorer lignes vides */
        if (*p == '\0') continue;

        /* ignorer commentaires complets */
        if (*p == '#') continue;

        /* autoriser commentaire en fin de ligne : couper à '#' */
        char *hash = strchr(p, '#');
        if (hash) *hash = '\0';

        /* tokenisation simple par espaces/tabs */
        char *save = NULL;
        char *tok = __strtok_r(p, " \t", &save);
        if (!tok) continue;

        char *tokens[4];
        int tcount = 0;
        while (tok && tcount < 4) {
            tokens[tcount++] = tok;
            tok = __strtok_r(NULL, " \t", &save);
        }

        /* si moins de 4 tokens, ligne mal formée -> ignorer */
        if (tcount < 4) continue;

        /* conversion simple des entiers avec validation basique */
        char *endptr;

        long arrival_l = strtol(tokens[1], &endptr, 10);
        if (endptr == tokens[1] || arrival_l < 0) continue;

        long exec_l = strtol(tokens[2], &endptr, 10);
        if (endptr == tokens[2] || exec_l <= 0) continue;

        long prio_l = strtol(tokens[3], &endptr, 10);
        if (endptr == tokens[3]) continue;

        /* garantir capacité */
        if (n >= cap) {
            size_t newcap = cap * 2;
            struct process *tmp = realloc(arr, newcap * sizeof *arr);
            if (!tmp) { free(arr); fclose(f); return -1; }
            arr = tmp;
            cap = newcap;
        }

        /* remplir la structure (assume struct process has fields: name, arrival, burst, priority, remaining, pid) */
        /* copier le nom en toute sécurité */
#ifdef NAME_LEN
        strncpy(arr[n].name, tokens[0], NAME_LEN - 1);
        arr[n].name[NAME_LEN - 1] = '\0';
#else
        strncpy(arr[n].name, tokens[0], sizeof(arr[n].name) - 1);
        arr[n].name[sizeof(arr[n].name) - 1] = '\0';
#endif
        arr[n].arrival_time = (int)arrival_l;
        arr[n].exec_time = (int)exec_l;
        arr[n].priority = (int)prio_l;
        arr[n].remaining_time = arr[n].exec_time;
        arr[n].status = 0;
        arr[n].end_time = 0;
        arr[n].waiting_time = 0;
        n++;
    }

    fclose(f);

    if (n == 0) {
        /* aucun processus valide trouvé */
        free(arr);
        *out = NULL;
        *out_n = 0;
        return 0; /* succès, mais 0 éléments */
    }

    /* réduire la mémoire allouée (optionnel) */
    {
        struct process *shrink = realloc(arr, n * sizeof *arr);
        if (shrink) arr = shrink;
    }

    *out = arr;
    *out_n = (int)n;
    return 0;
}
