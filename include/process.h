#ifndef PROCESS_H
#define PROCESS_H

#define NAME_LEN 64
#define READY 0
#define RUNNING 1
#define BLOCKED 2
#define ZOMBIE 3

/* Structure décrivant un processus */
struct process {
    char name[NAME_LEN];
    int arrival_time;    /* date d'arrivée (unités de temps, >=0) */
    int exec_time;   /* durée totale CPU requise (unités) */
    int priority;   /* priorité statique (plus petit = plus prioritaire) */
    int remaining_time;  /* temps restant (utilisé par ordonnanceurs) */
    int waiting_time;
    int status;
    int end_time;
    int wait_time;   // pour aging dynamique
};

#endif 
