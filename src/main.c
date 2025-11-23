#include <stdio.h>
#include <stdlib.h>
#include "../include/process.h"
#include "../include/parser.h"
#include "../include/scheduler.h"

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Usage : %s config.txt\n", argv[0]);
        return 1;
    }

    struct process *list = NULL;
    int n = 0;

    /* Charger le fichier config via parse_config_file */
    if (parse_config_file(argv[1], &list, &n) != 0) {
        printf("Erreur de parsing du fichier configuration.\n");
        return 1;
    }

    printf("%d processus chargés.\n", n);

    /* Charger dynamiquement les politiques */
    load_policies();

    /* Menu */
    int policy = choose_policy();

    /* Simulation */
    run_scheduler(list, n, policy);

    free(list);
    return 0;
}

