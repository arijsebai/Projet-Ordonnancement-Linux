#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>    /* for unlink, close */
#include <fcntl.h>     /* for mkstemp */
#include <sys/stat.h>
#include <sys/types.h>

#include "../include/process.h"

/* Prototype du parseur (implémentation dans src/parser.c) */
int parse_config(const char *path, struct process **out, int *out_n);

int main(int argc, char **argv) {
    if (argc < 2) return 1; /* attend un chemin de fichier en argument */

    const char *path = argv[1];
    struct process *procs = NULL;

    int n = 0;

    if (parse_config(path, &procs, &n) != 0) return 1;
    printf("Name\tArrival\tBurst\tPriority\tRemaining\twaiting\tstatus\tendtime\n");
    for (int i = 0; i < n; ++i) {
        printf("%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n",
               procs[i].name,
               procs[i].arrival_time,
               procs[i].exec_time,
               procs[i].priority,
               procs[i].remaining_time,
               procs[i].waiting_time,
               procs[i].status,
               procs[i].end_time);
}
free(procs);
return 0;
}
