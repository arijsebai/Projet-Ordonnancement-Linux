#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

// Scheduling function from policies/priority.c
int select_priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode);
extern int global_prio_mode;  // 1 = descending (default), 0 = ascending

// Compare processes for qsort (ready queue sorting)
int compare_priority(const void *a, const void *b) {
    const struct process *pa = a, *pb = b;
    return global_prio_mode ? (pb->priority - pa->priority) : (pa->priority - pb->priority);
}

int main() {
    // === HARDCODED PROCESSES (no config file needed) ===
    struct process procs[] = {
        {"P1", 0, 4, 3, 4, 0, 0, -1},
        {"P2", 1, 3, 1, 3, 0, 0, -1},
        {"P3", 2, 5, 2, 5, 0, 0, -1},
        {"P4", 3, 2, 5, 2, 0, 0, -1},
        {"P5", 5, 1, 1, 1, 0, 0, -1}
    };
    int n = 5;

    // === DISPLAY PROCESSES TABLE ===
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║               PROCESSES (HARDCODED DATA)                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name   Arrival  Exec  Priority\n");
    printf("  ─────  ───────  ────  ────────\n");
    for (int i = 0; i < n; i++) {
        printf("  %-6s %7d  %4d  %8d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].priority);
    }
    printf("\n");

    // === PRIORITY ORDER SELECTION ===
    char choice[10];
    int prio_mode = 1;  // default: descending

    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                 SELECT PRIORITY ORDERING                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  [1] DESCENDING → Higher number = Higher priority (DEFAULT)\n");
    printf("  [2] ASCENDING  → Lower number = Higher priority\n");
    printf("  Your choice (1 or 2) [Enter for default]: ");
    fflush(stdout);

    if (fgets(choice, sizeof(choice), stdin) && choice[0] == '2') {
        prio_mode = 0;
    }

    global_prio_mode = prio_mode;
    printf("\n  Selected: %s\n", prio_mode ? "DESCENDING" : "ASCENDING");
    printf("  Rule: Process with %s priority runs first.\n\n",
           prio_mode ? "highest" : "lowest");

    // === SIMULATION SETUP ===
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].status = 0;      // 0 = not arrived
        procs[i].end_time = -1;
    }

    int time = 0, completed = 0;
    struct process ready[100]; 
    int ready_n;

    // === SIMULATION (GANTT CHART) ===
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                     SIMULATION (GANTT CHART)                 ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Time  Executing  Ready Queue (sorted)\n");
    printf("  ─────  ─────────  ───────────────────────────────────────\n");

    while (completed < n) {
        // New arrivals
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time == time && procs[i].status == 0) {
                procs[i].status = 1;  // ready
            }
        }

        // Select next process
        int next = select_priority_preemptive(procs, n, time, -1, prio_mode);
        printf("  %5d  %-9s  [", time, next != -1 ? procs[next].name : "IDLE");

        // Build and sort ready queue
        ready_n = 0;
        for (int i = 0; i < n; i++) {
            if (procs[i].status == 1) {
                ready[ready_n++] = procs[i];
            }
        }
        if (ready_n > 1) {
            qsort(ready, ready_n, sizeof(struct process), compare_priority);
        }

        for (int i = 0; i < ready_n; i++) {
            if (i > 0) printf(", ");
            printf("%s:%d", ready[i].name, ready[i].priority);
        }
        printf("]\n");

        // Execute one time unit
        if (next != -1) {
            procs[next].remaining_time--;
            if (procs[next].remaining_time == 0) {
                procs[next].end_time = time + 1;
                procs[next].status = 2;  // completed
                completed++;
            }
        }
        time++;
    }

    // === FINAL STATISTICS ===
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                       FINAL STATISTICS                       ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name   Arrival  Exec  Finish  Wait\n");
    printf("  ─────  ───────  ────  ──────  ────\n");

    double total_wait = 0.0;
    for (int i = 0; i < n; i++) {
        int wait = procs[i].end_time - procs[i].arrival_time - procs[i].exec_time;
        total_wait += wait;
        printf("  %-6s %7d  %4d  %6d  %4d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].end_time, wait);
    }
    printf("  ─────────────────────────────────────\n");
    printf("  Average Wait Time: %.2f\n", total_wait / n);
    printf("  Total Time (Makespan): %d\n\n", time);

    printf("Simulation completed successfully.\n");
    return 0;
}