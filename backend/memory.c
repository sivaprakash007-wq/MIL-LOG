#include <stdio.h>
#include "memory.h"
#include "models.h"
#include "input.h"

#define MAX_BLOCKS 100
#define MAX_PROCESSES 200

typedef struct {
    int pid;
    int memory;
} MemoryProcess;

int loadMemoryProcesses(MemoryProcess p[]) {
    int n = 0;

    for (int i = 0; i < request_count; i++) {

        if (requests[i].status == 0) {

            p[n].pid = requests[i].id;
            p[n].memory = requests[i].memory_required;

            n++;
        }
    }

    return n;
}

void displayMemoryResult(
    MemoryProcess p[],
    int allocation[],
    int n,
    int blocks[],
    int block_count
) {
    printf("\n");
    printf("================================================================================\n");
    printf("                         MEMORY ALLOCATION RESULT                              \n");
    printf("================================================================================\n");

    printf("%-10s %-18s %-18s %-18s\n",
           "PID",
           "MEMORY REQUIRED",
           "BLOCK",
           "ALLOCATED SIZE");

    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {

        if (allocation[i] != -1) {

            printf("%-10d %-18d %-18d %-18d\n",
                   p[i].pid,
                   p[i].memory,
                   allocation[i] + 1,
                   blocks[allocation[i]]);
        }
        else {

            printf("%-10d %-18d %-18s %-18s\n",
                   p[i].pid,
                   p[i].memory,
                   "NOT ALLOCATED",
                   "-");
        }
    }

    printf("================================================================================\n");

    int total_memory = 0;
    int allocated_memory = 0;

    for (int i = 0; i < block_count; i++) {
        total_memory += blocks[i];
    }

    for (int i = 0; i < n; i++) {

        if (allocation[i] != -1) {
            allocated_memory += p[i].memory;
        }
    }

    printf("\nTotal Memory       : %d MB\n", total_memory);
    printf("Used Memory        : %d MB\n", allocated_memory);
    printf("Unused Memory      : %d MB\n",
           total_memory - allocated_memory);

    printf("\nBlock Status:\n");

    for (int i = 0; i < block_count; i++) {

        int used = 0;

        for (int j = 0; j < n; j++) {

            if (allocation[j] == i) {
                used = 1;
                break;
            }
        }

        if (used)
            printf("Block %d (%d MB) : ALLOCATED\n",
                   i + 1, blocks[i]);
        else
            printf("Block %d (%d MB) : FREE\n",
                   i + 1, blocks[i]);
    }
}

void getMemoryBlocks(int blocks[], int *block_count) {

    printf("\n========== MEMORY CONFIGURATION ==========\n");

    *block_count = readChoice("Enter number of memory blocks: ", 1, MAX_BLOCKS);

    for (int i = 0; i < *block_count; i++) {

        char prompt[80];
        snprintf(prompt, sizeof(prompt), "Enter size of Block %d (MB): ", i + 1);
        blocks[i] = readPositiveInt(prompt);
    }
}

void runFirstFit() {

    MemoryProcess p[MAX_PROCESSES];
    int blocks[MAX_BLOCKS];
    int allocation[MAX_PROCESSES];

    int block_count;

    int n = loadMemoryProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    getMemoryBlocks(blocks, &block_count);

    if (block_count == 0)
        return;

    for (int i = 0; i < n; i++)
        allocation[i] = -1;

    int occupied[MAX_BLOCKS] = {0};

    printf("\n========== FIRST FIT ==========\n");

    for (int i = 0; i < n; i++) {

        for (int j = 0; j < block_count; j++) {

            if (!occupied[j] &&
                blocks[j] >= p[i].memory) {

                allocation[i] = j;
                occupied[j] = 1;

                break;
            }
        }
    }

    displayMemoryResult(
        p,
        allocation,
        n,
        blocks,
        block_count
    );
}

void runBestFit() {

    MemoryProcess p[MAX_PROCESSES];
    int blocks[MAX_BLOCKS];
    int allocation[MAX_PROCESSES];

    int block_count;

    int n = loadMemoryProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    getMemoryBlocks(blocks, &block_count);

    if (block_count == 0)
        return;

    for (int i = 0; i < n; i++)
        allocation[i] = -1;

    int occupied[MAX_BLOCKS] = {0};

    printf("\n========== BEST FIT ==========\n");

    for (int i = 0; i < n; i++) {

        int best = -1;

        for (int j = 0; j < block_count; j++) {

            if (!occupied[j] &&
                blocks[j] >= p[i].memory) {

                if (best == -1 ||
                    blocks[j] < blocks[best]) {

                    best = j;
                }
            }
        }

        if (best != -1) {

            allocation[i] = best;
            occupied[best] = 1;
        }
    }

    displayMemoryResult(
        p,
        allocation,
        n,
        blocks,
        block_count
    );
}

void runWorstFit() {

    MemoryProcess p[MAX_PROCESSES];
    int blocks[MAX_BLOCKS];
    int allocation[MAX_PROCESSES];

    int block_count;

    int n = loadMemoryProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    getMemoryBlocks(blocks, &block_count);

    if (block_count == 0)
        return;

    for (int i = 0; i < n; i++)
        allocation[i] = -1;

    int occupied[MAX_BLOCKS] = {0};

    printf("\n========== WORST FIT ==========\n");

    for (int i = 0; i < n; i++) {

        int worst = -1;

        for (int j = 0; j < block_count; j++) {

            if (!occupied[j] &&
                blocks[j] >= p[i].memory) {

                if (worst == -1 ||
                    blocks[j] > blocks[worst]) {

                    worst = j;
                }
            }
        }

        if (worst != -1) {

            allocation[i] = worst;
            occupied[worst] = 1;
        }
    }

    displayMemoryResult(
        p,
        allocation,
        n,
        blocks,
        block_count
    );
}

void memoryMenu() {

    int choice;

    while (1) {

        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|          MEMORY MANAGEMENT           |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. First Fit                         |\n");
        printf("| 2. Best Fit                          |\n");
        printf("| 3. Worst Fit                         |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 3);

        switch (choice) {

            case 1:
                runFirstFit();
                break;

            case 2:
                runBestFit();
                break;

            case 3:
                runWorstFit();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}