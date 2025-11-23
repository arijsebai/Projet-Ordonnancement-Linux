#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void load_policies();
int choose_policy();
void run_scheduler(struct process *list, int n, int policy);

void fifo_simulation(struct process *p, int n);
void priority_simulation(struct process *p, int n, int prio_mode);
void rr_simulation(struct process *p, int n);

#endif

