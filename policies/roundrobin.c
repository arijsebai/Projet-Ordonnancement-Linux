#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

void round_robin(struct process *procs, int n, int quantum) {
    struct process *p = malloc(n * sizeof(struct process));
    memcpy(p, procs, n * sizeof(struct process));

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].waiting_time = 0;
        p[i].end_time = -1;
    }

    int time = 0;
    int completed = 0;

    int ready[100];
    int head = 0, tail = 0;

    printf("\n=== ROUND-ROBIN (Quantum = %d) ===\n", quantum);
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  --------------------------------\n");

    while (completed < n) {
        // Ajouter les processus arrivés
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && p[i].remaining_time > 0 && p[i].end_time == -1) {
                int in_queue = 0;
                for (int j = head; j < tail; j++) {
                    if (ready[j] == i) { in_queue = 1; break; }
                }
                if (!in_queue) {
                    ready[tail++] = i;
                }
            }
        }

        if (head == tail) {
            int next_arrival = -1;
            for (int i = 0; i < n; i++) {
                if (p[i].remaining_time > 0 && p[i].arrival_time > time) {
                    if (next_arrival == -1 || p[i].arrival_time < next_arrival)
                        next_arrival = p[i].arrival_time;
                }
            }
            if (next_arrival == -1) break;
            time = next_arrival;
            printf("%4d  [IDLE]       []\n", time);
            continue;
        }

        int curr = ready[head++];
        int run = (p[curr].remaining_time < quantum) ? p[curr].remaining_time : quantum;

        // Affichage
        printf("%4d  %-8s    [", time, p[curr].name);
        int first = 1;
        for (int j = head; j < tail; j++) {
            if (!first) printf(", ");
            printf("%s:%d", p[ready[j]].name, p[ready[j]].remaining_time);
            first = 0;
        }
        printf("]\n");

        // Incrémenter waiting_time pour tous en file (sauf le courant)
        for (int j = head; j < tail; j++) {
            p[ready[j]].waiting_time += run;
        }

        // Exécution
        p[curr].remaining_time -= run;
        time += run;

        // Ajouter les nouveaux arrivants pendant ce quantum
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time > time - run && p[i].arrival_time <= time &&
                p[i].remaining_time > 0 && p[i].end_time == -1) {
                int in_queue = 0;
                for (int j = head; j < tail; j++) {
                    if (ready[j] == i) { in_queue = 1; break; }
                }
                if (!in_queue) ready[tail++] = i;
            }
        }

        // Remettre en file si pas fini
        if (p[curr].remaining_time > 0) {
            ready[tail++] = curr;
        } else {
            p[curr].end_time = time;
            completed++;
        }
    }

    // STATISTIQUES
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");
    float total_wait = 0;
    int makespan = 0;
    for (int i = 0; i < n; i++) {
        int finish = p[i].end_time;
        int wait = finish - p[i].arrival_time - p[i].exec_time;  // Formule exacte
        printf("%-4s  %7d  %4d  %6d  %4d\n", p[i].name, p[i].arrival_time, p[i].exec_time, finish, wait);
        total_wait += wait;
        if (finish > makespan) makespan = finish;
    }
    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);

    free(p);
}
