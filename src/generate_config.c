/* generate_config.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int generate_config(const char *filename) {
    srand((unsigned)time(NULL));

    int nb, max_arrival, min_prio, max_prio, max_exec;

    printf("=== Automatic Process Configuration Generator ===\n");

    printf("Number of processes        : ");
    if (scanf("%d", &nb) != 1 || nb <= 0) return -1;

    printf("Maximum arrival time       : ");
    if (scanf("%d", &max_arrival) != 1 || max_arrival < 0) return -1;

    printf("Minimum priority           : ");
    if (scanf("%d", &min_prio) != 1) return -1;

    printf("Maximum priority           : ");
    if (scanf("%d", &max_prio) != 1 || max_prio < min_prio) return -1;

    printf("Maximum execution time     : ");
    if (scanf("%d", &max_exec) != 1 || max_exec <= 0) return -1;

    FILE *f = fopen(filename, "w");
    if (!f) {
        perror("Cannot create configuration file");
        return -1;
    }

    fprintf(f, "# Auto-generated file – %d random processes\n", nb);
    fprintf(f, "# Params: arrival[0-%d], priority[%d-%d], exec[1-%d]\n\n",
            max_arrival, min_prio, max_prio, max_exec);

    for (int i = 1; i <= nb; i++) {
        int arrival = rand() % (max_arrival + 1);
        int exec    = 1 + rand() % max_exec;
        int prio    = min_prio + rand() % (max_prio - min_prio + 1);

        fprintf(f, "P%d %d %d %d\n", i, arrival, exec, prio);
    }

    fclose(f);

    printf("\n✔ File '%s' generated successfully.\n", filename);
    return 0;
}
