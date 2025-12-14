#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define MAX_SEGMENTS 2048

struct gantt_segment {
	char process[NAME_LEN];
	int start;
	int end;
};

struct process_stat {
	char id[NAME_LEN];
	int arrival_time;
	int exec_time;
	int finish_time;
	int wait_time;
	int priority;
	int final_priority; 
};

struct simulation_result {
	char algorithm[64];
	struct gantt_segment segments[MAX_SEGMENTS];
	int segment_count;
	struct process_stat stats[256];
	int stat_count;
	double average_wait;
	int makespan;
};

struct scheduler_options {
	const char *algorithm;
	int quantum;
	int prio_mode; 
};

void load_policies();
int choose_policy();
void run_scheduler(struct process *list, int n, int policy);
int run_scheduler_api(struct process *list, int n, const struct scheduler_options *opts, struct simulation_result *out);
void print_json_result(const struct simulation_result *res);


void fifo_simulation(struct process *p, int n);
void priority_simulation(struct process *p, int n, int prio_mode);
void rr_simulation(struct process *p, int n);
void multilevel_simulation(struct process *p, int n, int quantum);
void multilevel_dynamic_simulation(struct process *procs, int n, int q);
void srt_simulation(struct process *procs, int n);
#endif

