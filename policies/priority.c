#include "../include/process.h"

int global_prio_mode = 1;  // ← SEULE DÉFINITION (OK)

int select_priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode) {
    (void)current;
    global_prio_mode = prio_mode;  // mise à jour

    int best = -1;
    int best_prio = (prio_mode == 1) ? -99999 : 99999;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            int p = procs[i].priority;
            if ((prio_mode == 1 && p > best_prio) || (prio_mode == 0 && p < best_prio)) {
                best_prio = p;
                best = i;
            }
        }
    }
    return best;
}
