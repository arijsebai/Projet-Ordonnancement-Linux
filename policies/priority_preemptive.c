#include "../include/process.h"

int global_prio_mode = 1;  


int priority_preemptive(struct process *procs, int n, int time, int current, int prio_mode) {
    (void)current; 
    global_prio_mode = prio_mode;

    int best = -1;
    int best_prio = prio_mode ? -99999 : 99999;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            int pr = procs[i].priority;
            if ((prio_mode && pr > best_prio) || (!prio_mode && pr < best_prio)) {
                best_prio = pr;
                best = i;
            }
        }
    }

    return best; 
}

