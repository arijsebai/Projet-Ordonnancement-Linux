
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

int select_multilevel(struct process *procs, int n, int time, int current, int quantum_expired);

// Fonction de comparaison pour qsort (pour l'affichage : Priorité Descendante)
int compare_priority_desc(const void *a, const void *b) {
    const struct process *pa = a, *pb = b;
    // Plus grande priorité en premier
    if (pb->priority != pa->priority) {
        return pb->priority - pa->priority;
    }
    // Si priorité égale, premier arrivé en premier
    return pa->arrival_time - pb->arrival_time;
}

int main() {
    // === CONFIGURATION SPÉCIFIQUE MULTILEVEL ===
    int quantum;
    printf("\n=== Configuration Multilevel ===\n");
    printf("Entrez la valeur du Quantum (pour Round-Robin sur même priorité) : ");
    if (scanf("%d", &quantum) != 1 || quantum <= 0) {
        quantum = 2; // Valeur par défaut de sécurité
        printf("Valeur invalide. Quantum par défaut = 2.\n");
    }

    // === PROCESSUS HARDCODÉS ===
    // On met des priorités variées pour tester le Multilevel
    struct process procs[] = {
        {"P1", 0, 4, 2, 4, 0, 0, -1}, // Prio Moyenne
        {"P2", 1, 6, 2, 6, 0, 0, -1}, // Prio Moyenne (Concurrence avec P1 -> RR)
        {"P3", 3, 2, 3, 2, 0, 0, -1}, // Prio Haute (Doit préempter)
        {"P4", 8, 3, 1, 3, 0, 0, -1}, // Prio Basse
        {"P5", 9, 4, 1, 2, 0, 0, -1}  // Prio Basse
    };
    int n = 5;

    // === AFFICHAGE DE LA TABLE DES PROCESSUS ===
    printf("\n╔══════════════════════════════════════════════════════════════╗\n");
    printf("║           PROCESSUS (DONNÉES DE TEST MULTILEVEL)             ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Name     Arrival   Exec   Priority\n");
    printf("  ─────    ───────   ────   ────────\n");
    for (int i = 0; i < n; i++) {
        printf("  %-6s %7d   %4d   %8d\n",
               procs[i].name, procs[i].arrival_time, procs[i].exec_time, procs[i].priority);
    }
    printf("\n");

    // === MISE EN PLACE DE LA SIMULATION ===
    for (int i = 0; i < n; i++) {
        procs[i].remaining_time = procs[i].exec_time;
        procs[i].status = 0;     // 0 = pas arrivé
        procs[i].end_time = -1;
    }

    int time = 0, completed = 0;
    struct process ready[100];  
    int ready_n;

    // Variables spécifiques au Multilevel/RR
    int current_proc = -1;
    int quantum_counter = 0;
    int quantum_expired = 0;

    // === SIMULATION (TABLEAU GANTT) ===
    printf("╔══════════════════════════════════════════════════════════════╗\n");
    printf("║              SIMULATION MULTILEVEL (TABLEAU GANTT)           ║\n");
    printf("╚══════════════════════════════════════════════════════════════╝\n");
    printf("  Time   Executing   Ready Queue (Trié par Prio:Nom)\n");
    printf("  ─────  ─────────   ───────────────────────────────────────\n");

    while (completed < n) {
        // 1. Nouveaux arrivés
        for (int i = 0; i < n; i++) {
            if (procs[i].arrival_time == time && procs[i].status == 0) {
                procs[i].status = 1;  // prêt
            }
        }

        // 2. Gestion du Quantum (Vérification avant sélection)
        if (current_proc != -1 && quantum_counter >= quantum) {
            quantum_expired = 1;
        } else {
            quantum_expired = 0;
        }

        // 3. Sélectionner le prochain processus
        int next = select_multilevel(procs, n, time, current_proc, quantum_expired);

        // 4. Gestion du changement de contexte pour le compteur Quantum
        if (next != current_proc) {
            quantum_counter = 0; // Reset si on change de processus
        }

        // 5. Affichage Gantt
        printf("  %5d  %-9s   [", time, next != -1 ? procs[next].name : "IDLE");

        // Construire et trier la file d'attente (pour l'affichage)
        ready_n = 0;
        for (int i = 0; i < n; i++) {
            // On affiche ceux qui sont prêts ET celui qui vient d'être éjecté par le quantum (si applicable)
            // Mais on n'affiche PAS celui qui est en train d'être choisi (next)
            if (procs[i].status == 1 && i != next) {
                ready[ready_n++] = procs[i];
            }
        }
        
        // Tri visuel : Priorité descendante
        if (ready_n > 1) {
            qsort(ready, ready_n, sizeof(struct process), compare_priority_desc);
        }

        for (int i = 0; i < ready_n; i++) {
            if (i > 0) printf(", ");
            printf("%s:%d", ready[i].name, ready[i].priority);
        }
        printf("]\n");

        // 6. Exécution
        if (next != -1) {
            procs[next].remaining_time--;
            quantum_counter++; // On consomme du quantum

            // Vérifier si le processus est terminé
            if (procs[next].remaining_time == 0) {
                procs[next].end_time = time + 1;
                procs[next].status = 2;  // terminé
                completed++;
                
                current_proc = -1; // Libérer CPU
                quantum_counter = 0;
            } else {
                current_proc = next; // Continuer ou prendre la main
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

    printf("Simulation MULTILEVEL terminée avec succès.\n");
    return 0;
}