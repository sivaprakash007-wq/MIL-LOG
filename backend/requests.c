#include <stdio.h>
#include <string.h>
#include "requests.h"
#include "units.h"
#include "inventory.h"
#include "storage.h"
#include "input.h"

#define PENDING 0
#define COMPLETED 1
#define CANCELLED 2

void createRequest() {
    if (request_count >= MAX_REQUESTS) {
        printf("\nMaximum number of requests reached.\n");
        return;
    }

    SupplyRequest r;

    printf("\n========== CREATE SUPPLY REQUEST ==========\n");

    r.id = readPositiveInt("Enter Request ID: ");

    for (int i = 0; i < request_count; i++) {
        if (requests[i].id == r.id) {
            printf("\nRequest ID already exists.\n");
            return;
        }
    }

    printf("\nAvailable Units:\n");

    if (unit_count == 0) {
        printf("No units registered. Add a unit first.\n");
        return;
    }

    displayUnits();

    r.unit_id = readPositiveInt("\nEnter Unit ID: ");

    int unit_found = 0;

    for (int i = 0; i < unit_count; i++) {
        if (units[i].id == r.unit_id) {
            unit_found = 1;
            break;
        }
    }

    if (!unit_found) {
        printf("\nInvalid Unit ID.\n");
        return;
    }

    if (equipment_count == 0) {
        printf("\nInventory is empty. Add equipment first.\n");
        return;
    }

    displayInventory();

    r.equipment_id = readPositiveInt("\nEnter Equipment ID: ");

    int equipment_found = -1;

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == r.equipment_id) {
            equipment_found = i;
            break;
        }
    }

    if (equipment_found == -1) {
        printf("\nInvalid Equipment ID.\n");
        return;
    }

    r.quantity = readPositiveInt("Enter Quantity Required: ");


    if (r.quantity > equipment[equipment_found].available) {
        printf("\nRequested quantity is greater than available quantity.\n");
        printf("Available quantity: %d\n",
               equipment[equipment_found].available);
        return;
    }

    printf("\nPriority:\n");
    printf("1. High\n");
    printf("2. Medium\n");
    printf("3. Low\n");
    r.priority = readChoice("Enter Priority: ", 1, 3);
    r.burst_time = readPositiveInt("\nEnter Burst Time: ");
    r.memory_required =
    readPositiveInt("Enter Memory Required (MB): ");

    r.status = PENDING;

    requests[request_count] = r;
    request_count++;

    saveRequests();

    printf("\n============================================\n");
    printf("       SUPPLY REQUEST CREATED SUCCESSFULLY\n");
    printf("============================================\n");
    printf("Request ID       : %d\n", r.id);
    printf("Unit ID          : %d\n", r.unit_id);
    printf("Equipment ID     : %d\n", r.equipment_id);
    printf("Quantity         : %d\n", r.quantity);
    printf("Priority         : %d\n", r.priority);
    printf("Burst Time       : %d\n", r.burst_time);
    printf("Memory Required  : %d MB\n", r.memory_required);
    printf("Status           : PENDING\n");
}

void displayRequests() {
    if (request_count == 0) {
        printf("\nNo supply requests available.\n");
        return;
    }

    printf("\n");
    printf("============================== SUPPLY REQUESTS ==============================\n");

    printf("%-8s %-10s %-12s %-10s %-10s %-10s %-12s\n",
           "ID", "UNIT", "EQUIPMENT", "QUANTITY",
           "PRIORITY", "MEMORY", "STATUS");

    printf("-----------------------------------------------------------------------------\n");

    for (int i = 0; i < request_count; i++) {

        char status[15];

        if (requests[i].status == PENDING)
            strcpy(status, "PENDING");
        else if (requests[i].status == COMPLETED)
            strcpy(status, "COMPLETED");
        else
            strcpy(status, "CANCELLED");

        printf("%-8d %-10d %-12d %-10d %-10d %-10dMB %-12s\n",
               requests[i].id,
               requests[i].unit_id,
               requests[i].equipment_id,
               requests[i].quantity,
               requests[i].priority,
               requests[i].memory_required,
               status);
    }

    printf("=============================================================================\n");
}

void searchRequest() {
    
    int id = readPositiveInt("\nEnter Request ID to search: ");

    for (int i = 0; i < request_count; i++) {

        if (requests[i].id == id) {

            printf("\n========== REQUEST FOUND ==========\n");

            printf("Request ID       : %d\n", requests[i].id);
            printf("Unit ID          : %d\n", requests[i].unit_id);
            printf("Equipment ID     : %d\n", requests[i].equipment_id);
            printf("Quantity         : %d\n", requests[i].quantity);
            printf("Priority         : %d\n", requests[i].priority);
            printf("Burst Time       : %d\n", requests[i].burst_time);
            printf("Memory Required  : %d MB\n",
                   requests[i].memory_required);

            if (requests[i].status == PENDING)
                printf("Status           : PENDING\n");
            else if (requests[i].status == COMPLETED)
                printf("Status           : COMPLETED\n");
            else
                printf("Status           : CANCELLED\n");

            return;
        }
    }

    printf("\nRequest not found.\n");
}

void cancelRequest() {
    int id = readPositiveInt("\nEnter Request ID to cancel: ");

    for (int i = 0; i < request_count; i++) {

        if (requests[i].id == id) {

            if (requests[i].status == COMPLETED) {
                printf("\nCompleted requests cannot be cancelled.\n");
                return;
            }

            if (requests[i].status == CANCELLED) {
                printf("\nRequest is already cancelled.\n");
                return;
            }

            requests[i].status = CANCELLED;

            saveRequests();

            printf("\nRequest cancelled successfully.\n");
            return;
        }
    }

    printf("\nRequest not found.\n");
}

void processRequest() {
    int id = readPositiveInt("\nEnter Request ID to process: ");

    for (int i = 0; i < request_count; i++) {

        if (requests[i].id == id) {

            if (requests[i].status == COMPLETED) {
                printf("\nRequest is already completed.\n");
                return;
            }

            if (requests[i].status == CANCELLED) {
                printf("\nCancelled request cannot be processed.\n");
                return;
            }

            int equipment_index = -1;

            for (int j = 0; j < equipment_count; j++) {
                if (equipment[j].id == requests[i].equipment_id) {
                    equipment_index = j;
                    break;
                }
            }

            if (equipment_index == -1) {
                printf("\nEquipment no longer exists.\n");
                return;
            }

            if (equipment[equipment_index].available <
                requests[i].quantity) {

                printf("\nInsufficient equipment available.\n");
                printf("Currently available: %d\n",
                       equipment[equipment_index].available);

                return;
            }

            equipment[equipment_index].available -=
                requests[i].quantity;

            requests[i].status = COMPLETED;

            saveInventory();
            saveRequests();

            printf("\n============================================\n");
            printf("          REQUEST PROCESSED\n");
            printf("============================================\n");

            printf("Request ID       : %d\n", requests[i].id);
            printf("Equipment ID     : %d\n", requests[i].equipment_id);
            printf("Quantity Issued  : %d\n", requests[i].quantity);
            printf("Status           : COMPLETED\n");

            return;
        }
    }

    printf("\nRequest not found.\n");
}

void saveRequests() {
    FILE *file = openDataFile("requests.dat", "wb");

    if (file == NULL) {
        printf("\nUnable to save request data.\n");
        return;
    }

    fwrite(&request_count, sizeof(int), 1, file);
    fwrite(requests, sizeof(SupplyRequest), request_count, file);

    fclose(file);
}

void loadRequests() {
    FILE *file = openDataFile("requests.dat", "rb");

    if (file == NULL) {
        return;
    }

    if (fread(&request_count, sizeof(int), 1, file) != 1) {
        request_count = 0;
        fclose(file);
        return;
    }

    if (request_count < 0 || request_count > MAX_REQUESTS) {
        request_count = 0;
        fclose(file);
        return;
    }

    if (fread(requests, sizeof(SupplyRequest), request_count, file) != (size_t)request_count)
        request_count = 0;

    fclose(file);
}

void requestMenu() {
    int choice;

    while (1) {

        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|          SUPPLY REQUESTS             |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. Create Supply Request             |\n");
        printf("| 2. Display All Requests              |\n");
        printf("| 3. Search Request                    |\n");
        printf("| 4. Cancel Request                    |\n");
        printf("| 5. Process Request                   |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 5);

        switch (choice) {

            case 1:
                createRequest();
                break;

            case 2:
                displayRequests();
                break;

            case 3:
                searchRequest();
                break;

            case 4:
                cancelRequest();
                break;

            case 5:
                processRequest();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}