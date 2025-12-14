#include "../include/process.h"
#include <limits.h>


int fifo_scheduler(struct process *procs, int n, int time, int current, int prio_mode) {
    (void)current;  
    (void)prio_mode; 

    int best_index = -1;
    int earliest_arrival = INT_MAX;

    for (int i = 0; i < n; i++) {
        if (procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
            if (procs[i].arrival_time < earliest_arrival) {
                earliest_arrival = procs[i].arrival_time;
                best_index = i;
            }
        }
    }

    return best_index; 
}

