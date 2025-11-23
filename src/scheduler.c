#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "../include/scheduler.h"
#include "../include/process.h"

/* prototypes policies */
extern int fifo_scheduler(struct process *, int, int, int, int);
extern int priority_preemptive(struct process *, int, int, int, int);
extern void round_robin(struct process *, int, int);

typedef struct {
    char name[100];
} Policy;

Policy policies[20];
int policy_count = 0;

/* scan du dossier policies */
void load_policies() {
    DIR *d = opendir("policies");
    if (!d) {
        perror("Impossible d’ouvrir policies/");
        exit(1);
    }

    struct dirent *file;
    while ((file = readdir(d)) != NULL) {
        if (strstr(file->d_name, ".c")) {
            strcpy(policies[policy_count].name, file->d_name);
            policy_count++;
        }
    }
    closedir(d);
}

/* menu dynamique */
int choose_policy() {
    printf("\n=== Choisir une politique ===\n");

    for (int i = 0; i < policy_count; i++)
        printf("%d - %s\n", i, policies[i].name);

    printf("Choix [Entrée pour FIFO par défaut] : ");

    char line[10];
    if (!fgets(line, sizeof(line), stdin) || line[0] == '\n') {
        // par défaut : FIFO
        for (int i = 0; i < policy_count; i++) {
            if (strstr(policies[i].name, "fifo"))
                return i;
        }
        return 0;
    }

    int c = atoi(line);
    if (c < 0 || c >= policy_count) return 0;
    return c;
}

/* lancement simulation */
void run_scheduler(struct process *list, int n, int policy) {

    if (strstr(policies[policy].name, "fifo"))
        fifo_simulation(list, n);

    else if (strstr(policies[policy].name, "priority")) {
        // Choix ASC ou DESC
        printf("Sélection de l’ordre de priorité:\n");
        printf("  [1] DESCENDING → Higher number = Higher priority (DEFAULT)\n");
        printf("  [2] ASCENDING  → Lower number = Higher priority\n");
        printf("Votre choix [Entrée pour DESC]: ");

        char choice[10];
        int prio_mode = 1; // default DESC
        if (fgets(choice, sizeof(choice), stdin) && choice[0] == '2') {
            prio_mode = 0;
        }

        printf("Ordre sélectionné: %s\n", prio_mode ? "DESCENDING" : "ASCENDING");

        priority_simulation(list, n, prio_mode);
    }

    else if (strstr(policies[policy].name, "round"))
        rr_simulation(list, n);
}

/* FIFO */
void fifo_simulation(struct process *p, int n) {
    printf("\n=== FIFO ===\n");
    printf("Time  Executing  Ready Queue\n");
    printf("----  ---------  ------------------------------\n");

    int finished = 0, time = 0;

    while (finished < n) {
        int idx = fifo_scheduler(p, n, time, -1, 0);

        if (idx == -1) {
            printf("%4d  %-9s  []\n", time, "IDLE");
            time++;
            continue;
        }

        printf("%4d  %-9s  [", time, p[idx].name);

        int first = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (!first) printf(", ");
                printf("%s:%d", p[i].name, p[i].remaining_time);
                first = 0;
            }
        }
        printf("]\n");

        // Execution
        time += p[idx].remaining_time;
        p[idx].remaining_time = 0;
        p[idx].end_time = time; // <-- finish time
        finished++;
    }

    // === FINAL STATISTICS ===
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");
    float total_wait = 0;
    int makespan = 0;
    for (int i = 0; i < n; i++) {
        int wait = p[i].end_time - p[i].arrival_time - p[i].exec_time;
        printf("%-4s  %7d  %4d  %6d  %4d\n",
               p[i].name, p[i].arrival_time, p[i].exec_time, p[i].end_time, wait);
        total_wait += wait;
        if (p[i].end_time > makespan) makespan = p[i].end_time;
    }
    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);
}


void priority_simulation(struct process *p, int n, int prio_mode) {
    printf("\n=== PRIORITY ===\n");
    printf("Time  Executing  Ready Queue\n");
    printf("----  ---------  ------------------------------\n");

    int finished = 0, time = 0;

    while (finished < n) {
        int idx = priority_preemptive(p, n, time, -1, prio_mode);

        if (idx == -1) {
            printf("%4d  %-9s  []\n", time, "IDLE");
            time++;
            continue;
        }

        printf("%4d  %-9s  [", time, p[idx].name);

        int first = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (!first) printf(", ");
                printf("%s:%d", p[i].name, p[i].remaining_time);
                first = 0;
            }
        }
        printf("]\n");

        p[idx].remaining_time--;
        // waiting time pour les autres
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0)
                p[i].waiting_time++;
        }

        time++;
        if (p[idx].remaining_time == 0) {
            p[idx].end_time = time;
            finished++;
        }
    }

    // === FINAL STATISTICS ===
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");
    float total_wait = 0;
    int makespan = 0;
    for (int i = 0; i < n; i++) {
        int wait = p[i].waiting_time; // déjà calculé
        printf("%-4s  %7d  %4d  %6d  %4d\n",
               p[i].name, p[i].arrival_time, p[i].exec_time, p[i].end_time, wait);
        total_wait += wait;
        if (p[i].end_time > makespan) makespan = p[i].end_time;
    }
    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);
}


/* Round robin */
void rr_simulation(struct process *p, int n) {
    int q;
    printf("Quantum: ");
    scanf("%d", &q);

    round_robin(p, n, q);
}

