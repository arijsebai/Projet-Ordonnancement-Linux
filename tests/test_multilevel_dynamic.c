#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

int select_multilevel_dynamic(struct process *procs, int n, int time, int current, int quantum_expired);

int compare_priority_desc(const void *a, const void *b) {
    const struct process *pa = a, *pb = b;
    // Plus grande priorité en premier
    if (pb->priority != pa->priority) {
        return pb->priority - pa->priority;
    }
    return pa->arrival_time - pb->arrival_time;
}

int main() {
    // === CONFIGURATION ===
    int quantum;
    printf("\n=== Simulation Multilevel Dynamique (Pénalité) ===\n");
    printf("Entrez la valeur du Quantum : ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        quantum = 2;
        printf("Valeur invalide. Défaut Q=2.\n");
    }

    // === PROCESSUS HARDCODÉS ===
    struct process procs[] = {
        {"P1", 0, 8, 4, 8, 0, 0, -1},
        {"P2", 1, 2, 4, 2, 0, 0, -1},
        {"P3", 4, 4, 3, 4, 0, 0, -1},
        {"P4", 6, 1, 5, 1, 0, 0, -1}, 
        {"P5", 8, 6, 2, 6, 0, 0, -1}  
    };
    int n = 5;

    // === AFFICHAGE TABLEAU INITIAL ===
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║         PROCESSUS (DONNÉES INITIALES DYNAMIQUES)             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name     Arrival   Exec   Priority (Init)\n");
    printf("  ─────    ───────   ────   ───────────────\n");
    for (int i = 0; i < n; i++) {
        printf("  %-6s %7d   %4d   %8d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].priority);
    }
    printf("\n");

    // Init Simulation
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].status = 0;
        procs[i].end_time = -1;
    }

    int time = 0, completed = 0;
    struct process ready[100];
    int ready_n;

    // Variables de gestion
    int current_proc = -1;
    int quantum_counter = 0;
    int quantum_expired = 0;

    // === SIMULATION (GANTT) ===
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           SIMULATION DYNAMIQUE (TABLEAU GANTT)               ║\n");
    printf("║      (Prio baisse de 1 si le quantum est consommé)           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Time   Executing   Prio   Ready Queue (Tri: Prio descendante)\n");
    printf("  ─────  ─────────   ────   ───────────────────────────────────\n");

    while (completed < n) {
        // 1. Arrivées
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time == time && procs[i].status == 0) {
                procs[i].status = 1;
            }
        }

        // 2. Vérification du Quantum et PÉNALITÉ DYNAMIQUE
        if (current_proc != -1 && quantum_counter >= quantum) {
            quantum_expired = 1;
            
            if (procs[current_proc].priority > 1) {
                procs[current_proc].priority--; // Baisse de 1
            }
            
            quantum_counter = 0; 
        } else {
            quantum_expired = 0;
        }

        int next = select_multilevel_dynamic(procs, n, time, current_proc, quantum_expired);

        if (next != current_proc) {
            quantum_counter = 0;
        }

        // 5. Affichage Gantt
        printf("  %5d  %-9s   %2d     [", time, 
               next != -1 ? procs[next].name : "IDLE",
               next != -1 ? procs[next].priority : 0);

        ready_n = 0;
        for (int i = 0; i < n; i++) {
            if (procs[i].status == 1 && i != next) ready[ready_n++] = procs[i];
        }
        if (ready_n > 1) qsort(ready, ready_n, sizeof(struct process), compare_priority_desc);

        for (int i = 0; i < ready_n; i++) {
            if (i > 0) printf(", ");
            printf("%s:%d", ready[i].name, ready[i].priority);
        }
        printf("]\n");

        // 6. Exécution
        if (next != -1) {
            procs[next].remaining_time--;
            quantum_counter++;

            if (procs[next].remaining_time == 0) {
                procs[next].end_time = time + 1;
                procs[next].status = 2; // Terminé
                completed++;
                
                current_proc = -1;
                quantum_counter = 0;
            } else {
                current_proc = next;
            }
        } else {
            current_proc = -1;
            quantum_counter = 0;
        }
        time++;
    }

    // === STATISTIQUES FINALES ===
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║                       STATISTIQUES FINALES                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name     Arrival   Exec   Finish   Wait   Final_Prio\n");
    printf("  ─────    ───────   ────   ──────   ────   ──────────\n");

    double total_wait = 0.0;
    for (int i = 0; i < n; i++) {
        int wait = procs[i].end_time - procs[i].arrival_time - procs[i].exec_time;
        total_wait += wait;
        printf("  %-6s %7d   %4d   %6d   %4d   %8d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, 
               procs[i].end_time, wait, procs[i].priority);
    }
    printf("  ────────────────────────────────────────────────────\n");
    printf("  Temps d'attente moyen: %.2f\n", total_wait / n);
    printf("  Temps total (Makespan): %d\n\n", time);

    return 0;
}