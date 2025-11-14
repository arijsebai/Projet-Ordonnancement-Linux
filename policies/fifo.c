#include "../include/process.h" 
#include <limits.h> 


int select_fifo(struct process *procs, int n, int time, int current, int prio_mode) {
    // Ces paramètres sont ignorés pour FIFO
    (void)current;
    (void)prio_mode;

    int best_index = -1;
    int earliest_arrival = INT_MAX; // On cherche le temps d'arrivée le plus bas

    for (int i = 0; i < n; i++) {
        // Le processus est-il "prêt" ?
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            
            if (procs[i].arrival_time < earliest_arrival) {
                earliest_arrival = procs[i].arrival_time;
                best_index = i;
            }
        }
    }

    // Retourne l'index du processus choisi (ou -1 si best_index n'a jamais changé)
    return best_index;
}