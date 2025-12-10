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
   Helpers for API/JSON mode
   ============================================================ */
static void copy_processes(struct process *dst, const struct process *src, int n) {
    memcpy(dst, src, (size_t)n * sizeof(struct process));
}

static void reset_process(struct process *p) {
    p->remaining_time = p->exec_time;
    p->waiting_time = 0;
    p->end_time = 0;
    p->status = 0;
}

static void add_segment(struct simulation_result *res, const char *name, int start, int duration) {
    if (duration <= 0 || res->segment_count >= MAX_SEGMENTS) return;

    /* Merge with previous segment if same process and contiguous */
    if (res->segment_count > 0) {
        struct gantt_segment *last = &res->segments[res->segment_count - 1];
        if (strcmp(last->process, name) == 0 && last->end == start) {
            last->end += duration;
            return;
        }
    }

    struct gantt_segment *seg = &res->segments[res->segment_count++];
    strncpy(seg->process, name, NAME_LEN - 1);
    seg->process[NAME_LEN - 1] = '\0';
    seg->start = start;
    seg->end = start + duration;
}

static void fill_stats_from_processes(const struct process *p, int n, struct simulation_result *res, const int *initial_priorities) {
    res->stat_count = 0;
    double total_wait = 0.0;
    int makespan = 0;

    for (int i = 0; i < n; i++) {
        if (res->stat_count >= 256) break;
        struct process_stat *s = &res->stats[res->stat_count++];
        strncpy(s->id, p[i].name, NAME_LEN - 1);
        s->id[NAME_LEN - 1] = '\0';
        s->arrival_time = p[i].arrival_time;
        s->exec_time = p[i].exec_time;
        s->finish_time = p[i].end_time;
        s->wait_time = p[i].end_time - p[i].arrival_time - p[i].exec_time;
        /* priority = valeur initiale si fournie, sinon la priorité actuelle */
        s->priority = initial_priorities ? initial_priorities[i] : p[i].priority;
        s->final_priority = p[i].priority; /* Default to current; multilevel_dynamic overrides later */
        total_wait += s->wait_time;
        if (p[i].end_time > makespan) makespan = p[i].end_time;
    }

    res->average_wait = (res->stat_count > 0) ? total_wait / res->stat_count : 0.0;
    res->makespan = makespan;
}

static void print_json_segments(const struct simulation_result *res) {
    printf("\"ganttData\":[");
    for (int i = 0; i < res->segment_count; i++) {
        const struct gantt_segment *g = &res->segments[i];
        int duration = g->end - g->start;
        printf("{\"process\":\"%s\",\"start\":%d,\"end\":%d,\"duration\":%d}",
               g->process, g->start, g->end, duration);
        if (i + 1 < res->segment_count) printf(",");
    }
    printf("]");
}

static void print_json_stats(const struct simulation_result *res) {
    printf(",\"processStats\":[");
    for (int i = 0; i < res->stat_count; i++) {
        const struct process_stat *s = &res->stats[i];
        int total_time = s->finish_time - s->arrival_time;
        printf("{\"id\":\"%s\",\"waitTime\":%d,\"totalTime\":%d,\"arrivalTime\":%d,\"executionTime\":%d,\"finishTime\":%d,\"priority\":%d",
               s->id, s->wait_time, total_time, s->arrival_time, s->exec_time, s->finish_time, s->priority);
        if (s->final_priority != s->priority) {
            printf(",\"finalPriority\":%d", s->final_priority);
        }
        printf("}");
        if (i + 1 < res->stat_count) printf(",");
    }
    printf("]");
}

void print_json_result(const struct simulation_result *res) {
    printf("{\"algorithm\":\"%s\",", res->algorithm);
    print_json_segments(res);
    print_json_stats(res);
    printf(",\"averageWait\":%.2f,\"makespan\":%d}\n", res->average_wait, res->makespan);
}

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
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  -----------------------------\n");

    int finished = 0, time = 0;

    while (finished < n) {
        int idx = fifo_scheduler(p, n, time, -1, 0);

        if (idx == -1) {
            printf("%4d  [IDLE]       []\n", time);
            time++;
            continue;
        }

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

        // Execute one second at a time and track waiting
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
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  -----------------------------\n");

    // Reinitialize metrics
    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].end_time = 0;
        p[i].waiting_time = 0;
    }

    int finished = 0, time = 0;

    while (finished < n) {
        int idx = priority_preemptive(p, n, time, -1, mode);

        if (idx == -1) {
            printf("%4d  [IDLE]       []\n", time);
            time++;
            continue;
        }

        printf("%4d  %-8s    [", time, p[idx].name);

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
    printf("Time  Executing     Ready Queue\n");
    printf("----  -----------  -----------------------------\n");

    while (finished < n) {
        int idx = select_multilevel(procs, n, time, current, quantum_counter >= quantum);

        if (idx == -1) {
            printf("%4d  [IDLE]       []\n", time);
            current = -1;
            quantum_counter = 0;
            time++;
            continue;
        }

        printf("%4d  %-8s    [", time, procs[idx].name);
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
                if (idx == -1 || p[i].remaining_time < min_rem ||
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

/* ============================================================
   Silent simulations for API/JSON mode
   ============================================================ */
static void fifo_simulation_collect(const struct process *src, int n, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);
    for (int i = 0; i < n; i++) reset_process(&p[i]);

    int finished = 0;
    int time = 0;
    res->segment_count = 0;

    while (finished < n) {
        int idx = fifo_scheduler(p, n, time, -1, 0);

        if (idx == -1) {
            time++;
            continue;
        }

        int run = p[idx].remaining_time;
        add_segment(res, p[idx].name, time, run);
        time += run;
        p[idx].remaining_time = 0;
        p[idx].end_time = time;
        finished++;
    }

    fill_stats_from_processes(p, n, res, NULL);
    free(p);
}

static void priority_simulation_collect(const struct process *src, int n, int mode, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);
    for (int i = 0; i < n; i++) reset_process(&p[i]);

    int finished = 0;
    int time = 0;
    res->segment_count = 0;

    while (finished < n) {
        int idx = priority_preemptive(p, n, time, -1, mode);

        if (idx == -1) {
            time++;
            continue;
        }

        add_segment(res, p[idx].name, time, 1);

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

    fill_stats_from_processes(p, n, res, NULL);
    free(p);
}

static void rr_simulation_collect(const struct process *src, int n, int quantum, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);
    for (int i = 0; i < n; i++) {
        reset_process(&p[i]);
        p[i].end_time = -1;
    }

    int ready[256];
    int head = 0, tail = 0;
    int time = 0;
    int completed = 0;
    res->segment_count = 0;

    while (completed < n) {
        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && p[i].remaining_time > 0 && p[i].end_time == -1) {
                int in_queue = 0;
                for (int j = head; j < tail; j++) {
                    if (ready[j] == i) { in_queue = 1; break; }
                }
                if (!in_queue) ready[tail++] = i;
            }
        }

        if (head == tail) {
            int next_arrival = -1;
            for (int i = 0; i < n; i++) {
                if (p[i].remaining_time > 0 && p[i].arrival_time > time) {
                    if (next_arrival == -1 || p[i].arrival_time < next_arrival)
                        next_arrival = p[i].arrival_time;
                }
            }
            if (next_arrival == -1) break;
            time = next_arrival;
            continue;
        }

        int curr = ready[head++];
        int run = (p[curr].remaining_time < quantum) ? p[curr].remaining_time : quantum;

        add_segment(res, p[curr].name, time, run);

        for (int j = head; j < tail; j++) {
            p[ready[j]].waiting_time += run;
        }

        p[curr].remaining_time -= run;
        time += run;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time > time - run && p[i].arrival_time <= time &&
                p[i].remaining_time > 0 && p[i].end_time == -1) {
                int in_queue = 0;
                for (int j = head; j < tail; j++) {
                    if (ready[j] == i) { in_queue = 1; break; }
                }
                if (!in_queue) ready[tail++] = i;
            }
        }

        if (p[curr].remaining_time > 0) {
            ready[tail++] = curr;
        } else {
            p[curr].end_time = time;
            completed++;
        }
    }

    fill_stats_from_processes(p, n, res, NULL);
    free(p);
}

static void multilevel_simulation_collect(const struct process *src, int n, int quantum, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);
    for (int i = 0; i < n; i++) reset_process(&p[i]);

    int finished = 0, time = 0;
    int current = -1;
    int quantum_counter = 0;
    res->segment_count = 0;

    while (finished < n) {
        int idx = select_multilevel(p, n, time, current, quantum_counter >= quantum);

        if (idx == -1) {
            current = -1;
            quantum_counter = 0;
            time++;
            continue;
        }

        add_segment(res, p[idx].name, time, 1);

        for (int i = 0; i < n; i++)
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0)
                p[i].waiting_time++;

        p[idx].remaining_time--;
        current = idx;
        quantum_counter++;

        if (p[idx].remaining_time == 0) {
            p[idx].end_time = time + 1;
            finished++;
            quantum_counter = 0;
        }

        if (quantum_counter >= quantum)
            quantum_counter = 0;

        time++;
    }

    fill_stats_from_processes(p, n, res, NULL);
    free(p);
}

static void multilevel_dynamic_simulation_collect(const struct process *src, int n, int quantum, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);

    int initial_priorities[256];
    for (int i = 0; i < n; i++) {
        reset_process(&p[i]);
        initial_priorities[i] = p[i].priority; // capture priorité initiale avant aging
    }

    int finished = 0, time = 0;
    int current = -1;
    int quantum_counter = 0;
    res->segment_count = 0;

    while (finished < n) {
        int idx = select_multilevel_dynamic(p, n, time, current, quantum_counter >= quantum);

        if (idx == -1) {
            quantum_counter = 0;
            current = -1;
            time++;
            continue;
        }

        add_segment(res, p[idx].name, time, 1);

        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0) {
                p[i].priority++;
                p[i].waiting_time++;
            }
        }

        p[idx].remaining_time--;
        current = idx;
        quantum_counter++;

        if (p[idx].remaining_time == 0) {
            p[idx].end_time = time + 1;
            finished++;
            quantum_counter = 0;
        }

        if (quantum_counter >= quantum)
            quantum_counter = 0;

        time++;
    }

    /* Fill stats and set final_priority for multilevel_dynamic */
    fill_stats_from_processes(p, n, res, initial_priorities);
    for (int i = 0; i < res->stat_count; i++) {
        res->stats[i].final_priority = p[i].priority;
    }
    free(p);
}

static void srt_simulation_collect(const struct process *src, int n, struct simulation_result *res) {
    struct process *p = malloc((size_t)n * sizeof(struct process));
    if (!p) return;
    copy_processes(p, src, n);

    for (int i = 0; i < n; i++) {
        p[i].remaining_time = p[i].exec_time;
        p[i].end_time = -1;
        p[i].waiting_time = 0;
    }

    int time = 0;
    int completed = 0;
    res->segment_count = 0;

    while (completed < n) {
        int idx = -1;
        int min_rem = 999999;

        for (int i = 0; i < n; i++) {
            if (p[i].arrival_time <= time && p[i].remaining_time > 0) {
                if (idx == -1 || p[i].remaining_time < min_rem ||
                    (p[i].remaining_time == min_rem && p[i].arrival_time < p[idx].arrival_time)) {
                    min_rem = p[i].remaining_time;
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            time++;
            continue;
        }

        add_segment(res, p[idx].name, time, 1);

        p[idx].remaining_time--;
        time++;

        if (p[idx].remaining_time == 0) {
            p[idx].end_time = time;
            completed++;
        }

        for (int i = 0; i < n; i++) {
            if (i != idx && p[i].arrival_time <= time && p[i].remaining_time > 0)
                p[i].waiting_time++;
        }
    }

    fill_stats_from_processes(p, n, res, NULL);
    free(p);
}

int run_scheduler_api(struct process *list, int n, const struct scheduler_options *opts, struct simulation_result *out) {
    if (!list || n <= 0 || !opts || !out) return -1;

    memset(out, 0, sizeof(*out));
    strncpy(out->algorithm, opts->algorithm ? opts->algorithm : "fifo", sizeof(out->algorithm) - 1);
    out->algorithm[sizeof(out->algorithm) - 1] = '\0';

    int quantum = opts->quantum > 0 ? opts->quantum : 2;
    int mode = opts->prio_mode;

    if (strstr(out->algorithm, "fifo")) {
        fifo_simulation_collect(list, n, out);
        return 0;
    }
    if (strstr(out->algorithm, "round")) {
        rr_simulation_collect(list, n, quantum, out);
        return 0;
    }
    if (strstr(out->algorithm, "multilevel_dynamic") || strstr(out->algorithm, "multilevel-dynamic")) {
        multilevel_dynamic_simulation_collect(list, n, quantum, out);
        return 0;
    }
    if (strstr(out->algorithm, "multilevel")) {
        multilevel_simulation_collect(list, n, quantum, out);
        return 0;
    }
    if (strstr(out->algorithm, "priority")) {
        priority_simulation_collect(list, n, mode, out);
        return 0;
    }
    if (strstr(out->algorithm, "srt") || strstr(out->algorithm, "sjf")) {
        srt_simulation_collect(list, n, out);
        return 0;
    }

    return -1;
}
