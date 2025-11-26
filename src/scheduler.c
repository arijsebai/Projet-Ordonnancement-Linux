#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include "../include/scheduler.h"
#include "../include/process.h"

/* ==== PROTOTYPES DES POLITIQUES ==== */
extern int fifo_scheduler(struct process *, int, int, int, int);
extern int priority_preemptive(struct process *, int, int, int, int);
extern void round_robin(struct process *, int, int);
extern void srt_simulation(struct process *, int);

/* Tes nouveaux algos */
extern int select_multilevel(struct process *, int, int, int, int);
extern int select_multilevel_dynamic(struct process *, int, int, int, int);

/* ==== LISTE DES POLITIQUES ==== */
typedef struct {
    char name[100];
} Policy;

Policy policies[20];
int policy_count = 0;

/* ============================================================
   ==========     CHARGEMENT DYNAMIQUE DES POLITIQUES     =====
   ============================================================ */
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

/* ===================== MENU ====================== */
int choose_policy() {
    printf("\n=== Choisir une politique ===\n");

    for (int i = 0; i < policy_count; i++)
        printf("%d - %s\n", i, policies[i].name);

    printf("Choix [Entrée → FIFO] : ");

    char line[10];
    if (!fgets(line, sizeof(line), stdin) || line[0] == '\n') {
        
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

/* ============================================================
                  SIMULATIONS DES POLITIQUES
   ============================================================ */

/* ---------------- FIFO ---------------- */
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

        time += p[idx].remaining_time;
        p[idx].remaining_time = 0;
        p[idx].end_time = time;
        finished++;
    }

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


 /* ---------------- PRIORITY ---------------- */
void priority_simulation(struct process *p, int n, int mode) {
    printf("\n=== PRIORITY ===\n");
    printf("Time  Executing  Ready Queue\n");
    printf("----  ---------  ------------------------------\n");

    int finished = 0, time = 0;

    while (finished < n) {
        int idx = priority_preemptive(p, n, time, -1, mode);

        if (idx == -1) {
            printf("%4d IDLE []\n", time);
            time++;
            continue;
        }

        printf("%4d %-10s [", time, p[idx].name);

        int f = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (!f) printf(", ");
                printf("%s:%d", p[i].name, p[i].remaining_time);
                f = 0;
            }
        }
        printf("]\n");

        p[idx].remaining_time--;
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

    // Stats
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");

    float tw = 0;
    int makespan = 0;

    for (int i = 0; i < n; i++) {
        printf("%-4s %7d %4d %6d %4d\n",
               p[i].name, p[i].arrival_time, p[i].exec_time,
               p[i].end_time, p[i].waiting_time);
        tw += p[i].waiting_time;
        if (p[i].end_time > makespan) makespan = p[i].end_time;
    }

    printf("\nAverage Wait Time: %.2f\n", tw / n);
    printf("Makespan: %d\n", makespan);
}


/* ---------------- ROUND ROBIN ---------------- */
void rr_simulation(struct process *p, int n) {
    int q;
    printf("Quantum: ");
    scanf("%d", &q);
    round_robin(p, n, q);
}


/* ============================================================
                        MULTI-LEVEL
   ============================================================ */
void multilevel_simulation(struct process *procs, int n, int quantum) {
    int finished = 0, time = 0;
    int current = -1;
    int quantum_counter = 0;

    printf("\n=== MULTI-LEVEL STATIC + AGING ===\n");

    while (finished < n) {
        int idx = select_multilevel(procs, n, time, current, quantum_counter >= quantum);

        if (idx == -1) {
            printf("%4d IDLE []\n", time);
            current = -1;
            quantum_counter = 0;
            time++;
            continue;
        }

        printf("%4d %-8s [", time, procs[idx].name);
        int f = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
                if (!f) printf(", ");
                printf("%s:%d", procs[i].name, procs[i].remaining_time);
                f = 0;
            }
        }
        printf("]\n");

        for (int i = 0; i < n; i++)
            if (i != idx && procs[i].arrival_time <= time && procs[i].remaining_time > 0)
                procs[i].waiting_time++;

        procs[idx].remaining_time--;
        current = idx;
        quantum_counter++;

        if (procs[idx].remaining_time == 0) {
            procs[idx].end_time = time + 1;
            finished++;
            quantum_counter = 0;
        }

        if (quantum_counter >= quantum)
            quantum_counter = 0;

        time++;
    }

    // Stats
    printf("\nFINAL STATISTICS\n");
    /* <-- MODIF : on affiche aussi la priorité finale ici */
    printf("Name  Arrival  Exec  Finish  Wait  Priority\n");
    float tw = 0;
    int makespan = 0;

    for (int i = 0; i < n; i++) {
        printf("%-4s %7d %4d %6d %4d %4d\n",
               procs[i].name, procs[i].arrival_time,
               procs[i].exec_time, procs[i].end_time,
               procs[i].waiting_time,
               procs[i].priority); /* ← priorité finale affichée */
        tw += procs[i].waiting_time;
        if (procs[i].end_time > makespan) makespan = procs[i].end_time;
    }

    printf("\nAverage Wait Time: %.2f\n", tw / n);
    printf("Makespan: %d\n", makespan);
}


 /* ============================================================
                    MULTI-LEVEL DYNAMIC
   ============================================================ */

void multilevel_dynamic_simulation(struct process *procs, int n, int quantum) {
    int finished = 0, time = 0;
    int current = -1;           
    int quantum_counter = 0;    

    printf("\n=== MULTI-LEVEL DYNAMIC ===\n");
    printf("Time  Executing  Ready Queue\n");
    printf("----  ---------  ------------------------------\n");

    while (finished < n) {
        int idx = select_multilevel_dynamic(procs, n, time, current, quantum_counter >= quantum);

        if (idx == -1) {
            printf("%4d  %-9s  []\n", time, "IDLE");
            time++;
            quantum_counter = 0;
            current = -1;
            continue;
        }

        printf("%4d  %-9s  [", time, procs[idx].name);
        int first = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
                if (!first) printf(", ");
                printf("%s:%d", procs[i].name, procs[i].remaining_time);
                first = 0;
            }
        }
        printf("]\n");

        /* Aging dynamique */
        for (int i = 0; i < n; i++) {
            if (i != idx && procs[i].arrival_time <= time && procs[i].remaining_time > 0) {
                procs[i].priority++;       // montée dynamique
                procs[i].waiting_time++;
            }
        }

        procs[idx].remaining_time--;
        current = idx;
        quantum_counter++;

        if (procs[idx].remaining_time == 0) {
            procs[idx].end_time = time + 1;
            finished++;
            quantum_counter = 0;
        }

        if (quantum_counter >= quantum)
            quantum_counter = 0;

        time++;
    }

    /* === STATISTIQUES FINALES AVEC PRIORITÉ === */
    float total_wait = 0;
    int makespan = 0;

    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait  Final_Prio\n");

    for (int i = 0; i < n; i++) {
        int wait = procs[i].waiting_time;
        printf("%-4s  %7d  %4d  %6d  %4d  %10d\n",
               procs[i].name,
               procs[i].arrival_time,
               procs[i].exec_time,
               procs[i].end_time,
               wait,
               procs[i].priority);  // ← ← ICI NOUVELLE PRIORITÉ
        total_wait += wait;

        if (procs[i].end_time > makespan)
            makespan = procs[i].end_time;
    }

    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);
}



 /* ============================================================
                     CHOIX DE LA POLITIQUE
   ============================================================ */
void run_scheduler(struct process *list, int n, int policy) {

    if (strstr(policies[policy].name, "fifo"))
        fifo_simulation(list, n);

    else if (strstr(policies[policy].name, "priority")) {
        printf("Ordre priorité: [1] DESC (defaut), [2] ASC: ");
        char line[10];
        int mode = 1;
        if (fgets(line, sizeof(line), stdin) && line[0] == '2')
            mode = 0;

        priority_simulation(list, n, mode);
    }

    else if (strstr(policies[policy].name, "round"))
        rr_simulation(list, n);

    else if (strstr(policies[policy].name, "multilevel_dynamic")) {
        int q;
        printf("Quantum: ");
        scanf("%d", &q);
        multilevel_dynamic_simulation(list, n, q);
    }
    else if (strstr(policies[policy].name, "srt"))
        srt_simulation(list, n);

    else if (strstr(policies[policy].name, "multilevel")) {
        int q;
        printf("Quantum: ");
        scanf("%d", &q);
        multilevel_simulation(list, n, q);
    }
}
void display_config_file(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Impossible d'ouvrir le fichier de configuration");
        return;
    }

    printf("\n=== Contenu du fichier de configuration (%s) ===\n", filename);

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    printf("============================================\n\n");

    fclose(f);
}
void srt_simulation(struct process *procs, int n) {
    // Create a copy to avoid modifying original processes
    struct process *p = malloc(n * sizeof(struct process));
    memcpy(p, procs, n * sizeof(struct process));

    // Initialize remaining time and end time
    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].end_time = -1;
        p[i].waiting_time = 0;
    }

    int time = 0;
    int completed = 0;

    printf("\n=== SHORTEST REMAINING TIME FIRST (SRT) ===\n");
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  -----------------------------\n");

    while (completed < n) {
        int idx = -1;
        int min_rem = 999999;

        // Find process with shortest remaining time that has arrived
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (p[i].remaining_time < min_rem ||
                    (p[i].remaining_time == min_rem && p[i].arrival_time < p[idx].arrival_time)) {
                    min_rem = p[i].remaining_time;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            // No process ready → IDLE
            printf("%4d  [IDLE]       []\n", time);
            time++;
            continue;
        }

        // Print executing process and ready queue
        printf("%4d  %-8s    [", time, p[idx].name);
        int first = 1;
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (!first) printf(", ");
                printf("%s:%d", p[i].name, p[i].remaining_time);
                first = 0;
            }
        }
        printf("]\n");

        // Execute 1 unit of time
        p[idx].remaining_time--;
        time++;

        // If finished
        if (p[idx].remaining_time == 0) {
            p[idx].end_time = time;
            completed++;
        }

        // Increase waiting time for other ready processes
        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0)
                p[i].waiting_time++;
        }
    }

    // Final statistics
    printf("\nFINAL STATISTICS\n");
    printf("Name  Arrival  Exec  Finish  Wait\n");

    float total_wait = 0.0f;
    int makespan = 0;

    for (int i = 0; i < n; i++) {
        int wait = p[i].waiting_time;
        total_wait += wait;
        if (p[i].end_time > makespan) makespan = p[i].end_time;

        printf("%-4s  %7d  %4d  %6d  %4d\n",
               p[i].name, p[i].arrival_time, p[i].exec_time,
               p[i].end_time, wait);
    }

    printf("\nAverage Wait Time: %.2f\n", total_wait / n);
    printf("Makespan: %d\n", makespan);

    free(p);
}
