
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"


void round_robin(struct process *procs, int n, int quantum);

int main() {
    
    struct process procs[] = {
        {"P1", 0, 4, 3, 4, 0, 0, -1},
        {"P2", 1, 3, 1, 3, 0, 0, -1},
        {"P3", 2, 5, 2, 5, 0, 0, -1},
        {"P4", 3, 2, 5, 2, 0, 0, -1},
        {"P5", 5, 1, 1, 1, 0, 0, -1}
    };
    int n = 5;

    
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║ PROCESSES                                                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf(" Name   Arrival  Exec  Priority\n");
    printf(" ─────  ───────  ────  ────────\n");
    for (int i = 0; i < n; i++) {
        printf(" %-6s %7d  %4d  %8d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].priority);
    }
    printf("\n");

    
    int quantum = 2;
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║ ENTER QUANTUM FOR ROUND-ROBIN                                ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf(" Your choice (e.g., 2) [Enter for default]: ");
    char input[10];
    fflush(stdout);
    if (fgets(input, sizeof(input), stdin)) {
        if (sscanf(input, "%d", &quantum) != 1 || quantum <= 0) {
            quantum = 2;
        }
    } else {
        quantum = 2;
    }
    printf("\n Selected quantum: %d\n\n", quantum);

    
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].end_time = -1;
    }

    
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║ SIMULATION (GANTT CHART)                                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf(" Time  Executing     Ready Queue (sorted)\n");
    printf(" ─────  ─────────  ───────────────────────────────────────\n");

    round_robin(procs, n, quantum);

    
    
    

    printf("\nSimulation completed successfully.\n");
    return 0;
}