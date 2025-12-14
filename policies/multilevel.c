
#include "../include/process.h"
#include <limits.h>

int select_multilevel(struct process *procs, int n, int time, int current, int quantum_expired) {
    int best_prio = -1;

    
    
    int processes_ready = 0;
    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].priority > best_prio) {
                best_prio = procs[i].priority;
            }
            processes_ready = 1;
        }
    }

    if (!processes_ready) return -1; 

    
    
    
    
    if (current != -1 && 
        procs[current].remaining_time > 0 && 
        procs[current].priority == best_prio && 
        procs[current].arrival_time <= time &&
        !quantum_expired) {
        return current;
    }

    
    
    int start_index = (current + 1) % n;
    
    for (int i = 0; i < n; i++) {
        int idx = (start_index + i) % n; 
        
        if (procs[idx].arrival_time <= time && 
            procs[idx].remaining_time > 0 && 
            procs[idx].priority == best_prio) {
            return idx;
        }
    }

    return -1;
}
