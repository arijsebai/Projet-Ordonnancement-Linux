#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/process.h"

void round_robin(struct process *procs, int n, int quantum) {
    struct process *p = malloc(n * sizeof(struct process));
    memcpy(p, procs, n * sizeof(struct process));

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].waiting_time = 0;
        p[i].end_time = -1;
    }

    int time = 0;
    int completed = 0;
    int current_idx = -1;
    int quantum_counter = 0;

    int ready[100];
    int head = 0, tail = 0;

    printf("\n=== ROUND-ROBIN (Quantum = %d) ===\n", quantum);
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  -----------------------------\n");

    while (completed < n) {
        
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time == time && p[i].remaining_time > 0) {
                ready[tail++] = i;
            }
        }

        
        if (current_idx == -1 || quantum_counter >= quantum || p[current_idx].remaining_time == 0) {
            if (current_idx != -1 && p[current_idx].remaining_time > 0) {
                
                ready[tail++] = current_idx;
            }

            if (head < tail) {
                current_idx = ready[head++];
                quantum_counter = 0;
            } else {
                current_idx = -1;
            }
        }

        if (current_idx == -1) {
            printf("%4d  [IDLE]       []\n", time);
            time++;
            continue;
        }

        
        printf("%4d  %-8s    [", time, p[current_idx].name);
        int first = 1;
        for (int j = head; j < tail; j++) {
            if (!first) printf(", ");
            printf("%s:%d", p[ready[j]].name, p[ready[j]].remaining_time);
            first = 0;
        }
        printf("]\n");

        
        p[current_idx].remaining_time--;
        quantum_counter++;

        
        for (int j = head; j < tail; j++) {
            p[ready[j]].waiting_time++;
        }

        time++;

        
        if (p[current_idx].remaining_time == 0) {
            p[current_idx].end_time = time;
            completed++;
            current_idx = -1;
            quantum_counter = 0;
        }
    }

    
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");
    float total_wait = 0;
    int makespan = 0;
    for (int i = 0; i < n; i++) {
        int finish = p[i].end_time;
        int wait = finish - p[i].arrival_time - p[i].exec_time;  
        printf("%-4s  %7d  %4d  %6d  %4d\n", p[i].name, p[i].arrival_time, p[i].exec_time, finish, wait);
        total_wait += wait;
        if (finish > makespan) makespan = finish;
    }
    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);

    free(p);
}
