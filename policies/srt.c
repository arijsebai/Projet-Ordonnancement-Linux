
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

void srtf(struct process *procs, int n) {
    struct process *p = malloc(n * sizeof(struct process));
    memcpy(p, procs, n * sizeof(struct process));

    // Initialiser
    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].end_time = -1;
    }

    int time = 0;
    int completed = 0;

    printf("\n=== SHORTEST REMAINING TIME FIRST (SRTF - Préemptif) ===\n");
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  --------------------------------\n");

    while (completed < n) {
        // 1. Trouver le processus avec le temps restant le plus court parmi ceux arrivés
        int best = -1;
        int min_rem = 999999;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (p[i].remaining_time < min_rem ||
                   (p[i].remaining_time == min_rem && p[i].arrival_time < p[best].arrival_time)) {
                    min_rem = p[i].remaining_time;
                    best = i;
                }
            }
        }

        // 2. Aucun processus prêt → IDLE
        if (best == -1) {
            printf("%4d  [IDLE]       []\n", time);
            time++;
            continue;
        }

        // 3. Affichage : état au début de l'unité de temps
        printf("%4d  %-8s    [", time, p[best].name);

        int first = 1;
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && 
                p[i].remaining_time > 0 && 
                i != best) {
                if (!first) printf(", ");
                printf("%s:%d", p[i].name, p[i].remaining_time);
                first = 0;
            }
        }
        printf("]\n");

        // 4. Exécuter 1 unité de temps
        p[best].remaining_time--;
        time++;

        // 5. Si terminé → marquer
        if (p[best].remaining_time == 0) {
            p[best].end_time = time;
            completed++;
        }
    }

    // === STATISTIQUES FINALES (identique à Round-Robin) ===
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");
    printf("----  -------  ----  ------  ----\n");

    float total_wait = 0.0f;
    int makespan = 0;

    for (int i = 0; i < n; i++) {
        int finish = p[i].end_time;
        int wait = finish - p[i].arrival_time - p[i].exec_time;
        total_wait += wait;
        if (finish > makespan) makespan = finish;

        printf("%-4s  %7d  %4d  %6d  %4d\n",
               p[i].name, p[i].arrival_time, p[i].exec_time, finish, wait);
    }

    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);

    free(p);
}
