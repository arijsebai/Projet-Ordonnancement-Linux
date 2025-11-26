#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    int nb, max_arrival, min_prio, max_prio, max_exec;

    printf("Number of processes        : ");
    scanf("%d", &nb);

    printf("Maximum arrival time       : ");
    scanf("%d", &max_arrival);

    printf("Minimum priority           : ");
    scanf("%d", &min_prio);

    printf("Maximum priority           : ");
    scanf("%d", &max_prio);

    printf("Maximum execution time     : ");
    scanf("%d", &max_exec);

    
    printf("\nGenerated processes:\n");
    printf("Name     Arrival   Exec   Prio\n");
    printf("--------------------------------\n");

    
    FILE *f = fopen("processes.conf", "w");
    fprintf(f, "# Fichier généré automatiquement - %d processus aléatoires\n", nb);
    fprintf(f, "# Paramètres : arrivée [0-%d], priorité [%d-%d], exécution [1-%d]\n\n",
            max_arrival, min_prio, max_prio, max_exec);

    for (int i = 1; i <= nb; i++) {
        int arrival = rand() % (max_arrival + 1);
        int exec    = 1 + rand() % max_exec;
        int prio    = min_prio + rand() % (max_prio - min_prio + 1);

        
        printf("P%-3d      %3d      %3d     %2d\n", i, arrival, exec, prio);

        
        fprintf(f, "P%d %d %d %d\n", i, arrival, exec, prio);
    }

    fclose(f);

    printf("\nFile 'processes.conf' generated successfully.\n");
    return 0;
}