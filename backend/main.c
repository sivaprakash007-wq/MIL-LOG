#include <stdio.h>
#include <stdlib.h>
#include "models.h"
#include "units.h"
#include "inventory.h"
#include "requests.h"
#include "scheduling.h"
#include "memory.h"
#include "paging.h"
#include "deadlock.h"
#include "filesystem.h"
#include "reports.h"
#include "input.h"

Unit units[MAX_UNITS];
Equipment equipment[MAX_EQUIPMENT];
SupplyRequest requests[MAX_REQUESTS];

int unit_count = 0;
int equipment_count = 0;
int request_count = 0;

void displayBanner() {
    printf("\n");
    printf("============================================================\n");
    printf("                      MIL-LOG v1.0                          \n");
    printf("        MILITARY LOGISTICS & OS RESOURCE SYSTEM            \n");
    printf("============================================================\n");
}

void mainMenu() {
    printf("\n");
    printf("+----------------------------------------------------------+\n");
    printf("|                     MAIN MENU                            |\n");
    printf("+----------------------------------------------------------+\n");
    printf("| 1. Unit Management                                       |\n");
    printf("| 2. Equipment & Inventory                                 |\n");
    printf("| 3. Supply Requests                                       |\n");
    printf("| 4. Process Scheduling                                    |\n");
    printf("| 5. Memory Management                                     |\n");
    printf("| 6. Virtual Memory / Paging                               |\n");
    printf("| 7. Resource Allocation & Deadlock                        |\n");
    printf("| 8. File Management                                       |\n");
    printf("| 9. Reports                                               |\n");
    printf("| 0. Exit                                                  |\n");
    printf("+----------------------------------------------------------+\n");
}

int main() {
    int choice;

    loadUnits();
    loadInventory();
    loadRequests();
    loadFiles();
    displayBanner();
    

    while (1) {
        mainMenu();

        choice = readChoice("Enter choice: ", 0, 9);

        switch (choice) {
            case 1:
                unitMenu();
                break;

            case 2:
                inventoryMenu();
                break;

            case 3:
                requestMenu();
                break;

            case 4:
                schedulingMenu();
                break;

            case 5:
                memoryMenu();
                break;

            case 6:
                pagingMenu();
                break;

            case 7:
                deadlockMenu();
                break;

            case 8:
                fileSystemMenu();
                break;

            case 9:
                reportsMenu();
                break;

            case 0:
                printf("\nShutting down MIL-LOG...\n");
                return 0;

            default:
                printf("\nInvalid choice. Please try again.\n");
        }
    }

    return 0;
}