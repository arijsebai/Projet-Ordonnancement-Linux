#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/utils.h"
#include "../include/process.h"
#include "../include/parser.h"
#include "../include/scheduler.h"
#include "../include/generate_config.h"

static const char *default_config = "config/sample_config.txt";

static void usage(void) {
    printf("Usage: ordonnanceur [--api --config <file> --algo <name> --quantum <q> --prio-order <asc|desc>]\n");
}

int main(int argc, char **argv) {
    char filename[256] = "";       // Full path
    char buffer[32];
    int choice = 1;  // Default: generate configuration
    int direct_file_mode = 0;     // If user provides a file directly as first arg

    /* CLI flags to support JSON/API mode */
    int api_mode = 0;
    int parse_only = 0;
    char algo[64] = "fifo";
    int quantum = 2;
    int prio_mode = 1; // 1 = descending (default)

    // If user provides a file path as the first argument (not a flag), use it directly
    if (argc == 2 && argv[1][0] != '-') {
        strncpy(filename, argv[1], sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        direct_file_mode = 1;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--api") == 0) {
            api_mode = 1;
        } else if (strcmp(argv[i], "--parse-config") == 0 && i + 1 < argc) {
            parse_only = 1;
            strncpy(filename, argv[++i], sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
        } else if (strcmp(argv[i], "--config") == 0 && i + 1 < argc) {
            strncpy(filename, argv[++i], sizeof(filename) - 1);
            filename[sizeof(filename) - 1] = '\0';
        } else if (strcmp(argv[i], "--algo") == 0 && i + 1 < argc) {
            strncpy(algo, argv[++i], sizeof(algo) - 1);
            algo[sizeof(algo) - 1] = '\0';
        } else if (strcmp(argv[i], "--quantum") == 0 && i + 1 < argc) {
            quantum = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--prio-order") == 0 && i + 1 < argc) {
            const char *val = argv[++i];
            prio_mode = (strcmp(val, "asc") == 0) ? 0 : 1;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage();
            return 0;
        }
    }

    if (parse_only) {
        struct process *list = NULL;
        int n = 0;

        if (parse_config_file(filename, &list, &n) != 0 || n == 0) {
            fprintf(stderr, "{\"error\":\"Failed to parse config file\"}\n");
            return 1;
        }

        printf("[");
        for (int i = 0; i < n; i++) {
            printf("{\"id\":\"%s\",\"arrivalTime\":%d,\"executionTime\":%d,\"priority\":%d}",
                   list[i].name, list[i].arrival_time, list[i].exec_time, list[i].priority);
            if (i + 1 < n) printf(",");
        }
        printf("]\n");
        free(list);
        return 0;
    }

    if (api_mode) {
        const char *cfg = (filename[0] != '\0') ? filename : default_config;
        struct process *list = NULL;
        int n = 0;

        if (parse_config_file(cfg, &list, &n) != 0 || n == 0) {
            fprintf(stderr, "Failed to load configuration file %s\n", cfg);
            return 1;
        }

        struct scheduler_options opts = { algo, quantum, prio_mode };
        struct simulation_result res;
        if (run_scheduler_api(list, n, &opts, &res) != 0) {
            fprintf(stderr, "Scheduler API failed\n");
            free(list);
            return 1;
        }

        print_json_result(&res);
        free(list);
        return 0;
    }

    // If a file was provided directly, skip the interactive menu
    if (!direct_file_mode) {
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
            // Clear stdin buffer after scanf
            int c; while ((c = getchar()) != '\n' && c != EOF);
        }
    }

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

