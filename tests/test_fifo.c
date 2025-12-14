#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h" 


int fifo_scheduler(struct process *procs, int n, int time, int current, int prio_mode);


int compare_arrival(const void *a, const void *b) {
    const struct process *pa = a, *pb = b;
    return (pa->arrival_time - pb->arrival_time);
}

int main() {
    
    struct process procs[] = {
        {"P1", 0, 4, 0, 4, 0, 0, -1},
        {"P2", 1, 3, 0, 3, 0, 0, -1},
        {"P3", 2, 5, 0, 5, 0, 0, -1},
        {"P4", 3, 2, 0, 2, 0, 0, -1},
        {"P5", 5, 1, 0, 1, 0, 0, -1}
    };
    int n = 5;

    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 PROCESSUS (DONNÉES DE TEST FIFO)             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name     Arrival   Exec\n");
    printf("  ─────    ───────   ────\n");
    for (int i = 0; i < n; i++) {
        printf("  %-6s %7d   %4d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time);
    }
    printf("\n");

    
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].status = 0;     
        procs[i].end_time = -1;
    }

    int time = 0, completed = 0;
    struct process ready[100];  
    int ready_n;

    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SIMULATION FIFO (TABLEAU GANTT)              ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Time   Executing   Ready Queue (Ordre Arrivée: P:Arr)\n");
    printf("  ─────  ─────────   ───────────────────────────────────────\n");

    while (completed < n) {
        
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time == time && procs[i].status == 0) {
                procs[i].status = 1;  
            }
        }

        
        
        int next = fifo_scheduler(procs, n, time, -1, 0); 
        
        printf("  %5d  %-9s   [", time, next != -1 ? procs[next].name : "IDLE");

        
        ready_n = 0;
        for (int i = 0; i < n; i++) {
            if (procs[i].status == 1 && i != next) { 
                ready[ready_n++] = procs[i];
            }
        }
        if (ready_n > 1) {
            qsort(ready, ready_n, sizeof(struct process), compare_arrival);
        }

        for (int i = 0; i < ready_n; i++) {
            if (i > 0) printf(", ");
            printf("%s:%d", ready[i].name, ready[i].arrival_time);
        }
        printf("]\n");

        
        if (next != -1) {
            procs[next].remaining_time--;
            
            
            if (procs[next].remaining_time == 0) {
                procs[next].end_time = time + 1;
                procs[next].status = 2;  
                completed++;
            }
        }
        time++;
    }

    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                       STATISTIQUES FINALES                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name     Arrival   Exec   Finish   Wait\n");
    printf("  ─────    ───────   ────   ──────   ────\n");

    double total_wait = 0.0;
    for (int i = 0; i < n; i++) {
        int wait = procs[i].end_time - procs[i].arrival_time - procs[i].exec_time;
        total_wait += wait;
        printf("  %-6s %7d   %4d   %6d   %4d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].end_time, wait);
    }
    printf("  ─────────────────────────────────────\n");
    printf("  Temps d'attente moyen: %.2f\n", total_wait / n);
    printf("  Temps total (Makespan): %d\n\n", time);

    printf("Simulation FIFO terminée avec succès.\n");
    return 0;
}
