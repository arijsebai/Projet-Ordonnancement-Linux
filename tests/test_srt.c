#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

// Fonction SRTF depuis policies/srtf.c
void srtf(struct process *procs, int n);

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
    printf("║ PROCESSES                                                    ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf(" Name   Arrival  Exec  Priority\n");
    printf(" ─────  ───────  ────  ────────\n");
    for (int i = 0; i < n; i++) {
        printf(" %-6s %7d  %4d  %8d\n",
               procs[i].name, procs[i].arrival_time,
               procs[i].exec_time, procs[i].priority);
    }
    printf("\n");

    // === SIMULATION (GANTT CHART) ===
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║ SIMULATION (GANTT CHART)                                     ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf(" Time  Executing     Ready Queue (sorted by remaining time)\n");
    printf(" ─────  ─────────  ───────────────────────────────────────────────\n");

    // Initialisation des champs nécessaires
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].end_time = -1;
    }

    // Lancement direct de SRTF (préemptif pur – pas de quantum)
    srtf(procs, n);

    // Message final discret (comme dans ton Round-Robin)
    printf("\nSimulation completed successfully.\n");

    return 0;
}