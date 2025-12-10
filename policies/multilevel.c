
#include "../include/process.h"
#include <limits.h>

int select_multilevel(struct process *procs, int n, int time, int current, int quantum_expired) {
    int best_prio = -1;

    // Étape 1 : Trouver la priorité la plus haute disponible (Processus prêts)
    // On suppose ici : Grande valeur = Haute Priorité (ex: 10 > 1)
    int processes_ready = 0;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].priority > best_prio) {
                best_prio = procs[i].priority;
            }
            processes_ready = 1;
        }
    }

    if (!processes_ready) return -1; // CPU IDLE

    // Étape 2 : Gestion du Round-Robin pour cette priorité 'best_prio'
    
    // Si on a un processus en cours, qu'il est toujours prêt, qu'il a toujours
    // la meilleure priorité, et que son quantum N'EST PAS fini... on le garde !
    if (current != -1 && 
        procs[current].remaining_time > 0 && 
        procs[current].priority == best_prio && 
        procs[current].arrival_time <= time &&
        !quantum_expired) {
        return current;
    }

    // Étape 3 : Sinon, on cherche le PROCHAIN candidat avec 'best_prio'
    // On commence la recherche juste après 'current' pour assurer le tour de rôle (Round Robin)
    int start_index = (current + 1) % n;
    
    for (int i = 0; i < n; i++) {
        int idx = (start_index + i) % n; // Modulo pour boucle circulaire
        
        if (procs[idx].arrival_time <= time && 
            procs[idx].remaining_time > 0 && 
            procs[idx].priority == best_prio) {
            return idx;
        }
    }

    return -1;
}
