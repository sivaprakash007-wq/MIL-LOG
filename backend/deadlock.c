#include <stdio.h>
#include "deadlock.h"
#include "input.h"

#define MAX_PROCESSES 20
#define MAX_RESOURCES 10

void displayMatrix(int matrix[][MAX_RESOURCES],
                   int processes,
                   int resources,
                   const char *title) {

    printf("\n%s\n", title);

    printf("        ");

    for (int j = 0; j < resources; j++) {
        printf("R%-7d", j + 1);
    }

    printf("\n");

    for (int i = 0; i < processes; i++) {

        printf("P%-6d", i + 1);

        for (int j = 0; j < resources; j++) {
            printf("%-8d", matrix[i][j]);
        }

        printf("\n");
    }
}

void runBankersAlgorithm() {

    int processes;
    int resources;

    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int maximum[MAX_PROCESSES][MAX_RESOURCES];
    int need[MAX_PROCESSES][MAX_RESOURCES];

    int available[MAX_RESOURCES];

    int finish[MAX_PROCESSES] = {0};
    int safe_sequence[MAX_PROCESSES];

    printf("\n");
    printf("====================================================\n");
    printf("             BANKER'S ALGORITHM                     \n");
    printf("====================================================\n");

    printf("\nMilitary Resource Types:\n");
    printf("R1 = Vehicles\n");
    printf("R2 = Personnel\n");
    printf("R3 = Fuel Units\n");
    printf("R4 = Repair Bays\n");

    processes = readChoice("\nEnter number of processes/units: ", 1, MAX_PROCESSES);

    resources = readChoice("Enter number of resource types: ", 1, MAX_RESOURCES);

    printf("\nEnter AVAILABLE resources:\n");

    for (int j = 0; j < resources; j++) {

        char prompt[80];
        snprintf(prompt, sizeof(prompt), "R%d available: ", j + 1);
        available[j] = readNonNegativeInt(prompt);
    }

    printf("\n========== CURRENT ALLOCATION ==========\n");

    for (int i = 0; i < processes; i++) {

        printf("\nProcess/Unit P%d\n", i + 1);

        for (int j = 0; j < resources; j++) {

            char prompt[80];
            snprintf(prompt, sizeof(prompt), "Allocated R%d: ", j + 1);
            allocation[i][j] = readNonNegativeInt(prompt);
        }
    }

    printf("\n========== MAXIMUM RESOURCE NEED ==========\n");

    for (int i = 0; i < processes; i++) {

        printf("\nMaximum requirement for P%d\n", i + 1);

        for (int j = 0; j < resources; j++) {

            char prompt[80];
            snprintf(prompt, sizeof(prompt), "Maximum R%d: ", j + 1);
            maximum[i][j] = readNonNegativeInt(prompt);

            if (maximum[i][j] < allocation[i][j]) {

                printf("\nMaximum requirement cannot be less than");
                printf(" current allocation.\n");

                return;
            }
        }
    }

    /*
     * Calculate Need matrix.
     *
     * Need = Maximum - Allocation
     */

    for (int i = 0; i < processes; i++) {

        for (int j = 0; j < resources; j++) {

            need[i][j] =
                maximum[i][j] - allocation[i][j];
        }
    }

    displayMatrix(
        allocation,
        processes,
        resources,
        "\nALLOCATION MATRIX"
    );

    displayMatrix(
        maximum,
        processes,
        resources,
        "\nMAXIMUM MATRIX"
    );

    displayMatrix(
        need,
        processes,
        resources,
        "\nNEED MATRIX"
    );

    printf("\nAVAILABLE RESOURCES:\n");

    for (int j = 0; j < resources; j++) {
        printf("R%d = %d   ",
               j + 1,
               available[j]);
    }

    printf("\n");

    /*
     * Banker's safety algorithm.
     */

    int count = 0;

    while (count < processes) {

        int found = 0;

        for (int i = 0; i < processes; i++) {

            if (finish[i])
                continue;

            int possible = 1;

            for (int j = 0; j < resources; j++) {

                if (need[i][j] > available[j]) {

                    possible = 0;
                    break;
                }
            }

            if (possible) {

                /*
                 * Pretend P_i finishes and releases
                 * its currently allocated resources.
                 */

                for (int j = 0; j < resources; j++) {

                    available[j] +=
                        allocation[i][j];
                }

                safe_sequence[count] = i;

                finish[i] = 1;

                count++;
                found = 1;
            }
        }

        /*
         * No process can safely execute.
         * Therefore the state is unsafe.
         */

        if (!found)
            break;
    }

    if (count == processes) {

        printf("\n");
        printf("====================================================\n");
        printf("                 SYSTEM IS SAFE                     \n");
        printf("====================================================\n");

        printf("\nSafe Sequence:\n");

        for (int i = 0; i < processes; i++) {

            printf("P%d", safe_sequence[i] + 1);

            if (i != processes - 1)
                printf(" -> ");
        }

        printf("\n");

        printf("\nAll resource requests can be satisfied");
        printf(" without entering a deadlock state.\n");
    }
    else {

        printf("\n");
        printf("====================================================\n");
        printf("                SYSTEM IS UNSAFE                    \n");
        printf("====================================================\n");

        printf("\nA safe sequence could not be found.\n");
        printf("Deadlock may occur if the current allocation continues.\n");
    }
}

void runDeadlockDetection() {

    int processes;
    int resources;

    int allocation[MAX_PROCESSES][MAX_RESOURCES];
    int request[MAX_PROCESSES][MAX_RESOURCES];

    int available[MAX_RESOURCES];

    int finish[MAX_PROCESSES] = {0};

    printf("\n");
    printf("====================================================\n");
    printf("             DEADLOCK DETECTION                     \n");
    printf("====================================================\n");

    processes = readChoice("\nEnter number of processes/units: ", 1, MAX_PROCESSES);

    resources = readChoice("Enter number of resource types: ", 1, MAX_RESOURCES);

    printf("\nEnter AVAILABLE resources:\n");

    for (int j = 0; j < resources; j++) {

        char prompt[80];
        snprintf(prompt, sizeof(prompt), "R%d available: ", j + 1);
        available[j] = readNonNegativeInt(prompt);
    }

    printf("\n========== CURRENT ALLOCATION ==========\n");

    for (int i = 0; i < processes; i++) {

        printf("\nProcess P%d\n", i + 1);

        for (int j = 0; j < resources; j++) {

            char prompt[80];
            snprintf(prompt, sizeof(prompt), "Allocated R%d: ", j + 1);
            allocation[i][j] = readNonNegativeInt(prompt);
        }
    }

    printf("\n========== CURRENT REQUEST ==========\n");

    for (int i = 0; i < processes; i++) {

        printf("\nRemaining request for P%d\n", i + 1);

        for (int j = 0; j < resources; j++) {

            char prompt[80];
            snprintf(prompt, sizeof(prompt), "Request R%d: ", j + 1);
            request[i][j] = readNonNegativeInt(prompt);
        }
    }

    int count = 0;

    while (count < processes) {

        int found = 0;

        for (int i = 0; i < processes; i++) {

            if (finish[i])
                continue;

            int possible = 1;

            for (int j = 0; j < resources; j++) {

                if (request[i][j] > available[j]) {

                    possible = 0;
                    break;
                }
            }

            if (possible) {

                for (int j = 0; j < resources; j++) {

                    available[j] +=
                        allocation[i][j];
                }

                finish[i] = 1;

                count++;
                found = 1;
            }
        }

        if (!found)
            break;
    }

    if (count == processes) {

        printf("\n============================================\n");
        printf("       NO DEADLOCK DETECTED\n");
        printf("============================================\n");

        printf("\nAll processes can eventually complete.\n");
    }
    else {

        printf("\n============================================\n");
        printf("          DEADLOCK DETECTED\n");
        printf("============================================\n");

        printf("\nProcesses involved:\n");

        for (int i = 0; i < processes; i++) {

            if (!finish[i])
                printf("P%d ", i + 1);
        }

        printf("\n");
    }
}

void deadlockMenu() {

    int choice;

    while (1) {

        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|      RESOURCE ALLOCATION &           |\n");
        printf("|             DEADLOCK                 |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. Banker's Algorithm                |\n");
        printf("| 2. Deadlock Detection                |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 2);

        switch (choice) {

            case 1:
                runBankersAlgorithm();
                break;

            case 2:
                runDeadlockDetection();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}