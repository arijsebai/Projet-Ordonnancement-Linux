#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"
#include "../include/process.h"
#include "../include/parser.h"
#include "../include/scheduler.h"
#include "../include/generate_config.h"

int main() {
    char filename[256];       // Full path
    char buffer[32];
    int choice = 1;  // Default: generate configuration

    printf("=== Scheduler Project ===\n");
    printf("1) Generate configuration file automatically  (default)\n");
    printf("2) Use an existing configuration file\n");
    printf("Your choice (press ENTER for default): ");

    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] != '\n') {
            int temp = atoi(buffer);
            if (temp == 1 || temp == 2)
                choice = temp;
            else
                printf("⚠ Invalid input -> Default = 1\n");
        }
    }

    if (choice == 1) {
        // Use timestamp to generate unique name
        time_t t = time(NULL);
        struct tm *tm_info = localtime(&t);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", tm_info);

        // Prepare full path in config folder with .txt extension
        snprintf(filename, sizeof(filename), "config/sample_config_%s.txt", timestamp);

        printf("\n➡ Generating configuration file: %s\n", filename);

        if (generate_config(filename) != 0) {
            printf("❌ Error generating configuration file.\n");
            return 1;
        }
    }
    else {
        printf("Enter configuration file name (with path if needed): ");
        scanf("%255s", filename);
    }

    // Clear stdin buffer before using other inputs
    int c; while ((c = getchar()) != '\n' && c != EOF);

    struct process *list = NULL;
    int n = 0;

    printf("\nLoading configuration file: %s\n", filename);
    display_config_file(filename);

    if (parse_config_file(filename, &list, &n) != 0) {
        printf("❌ Error parsing configuration file.\n");
        return 1;
    }

    printf("✔ %d processes loaded.\n", n);

    load_policies();
    int policy = choose_policy();

    run_scheduler(list, n, policy);

    free(list);
    return 0;
}

