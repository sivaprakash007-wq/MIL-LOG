#include <stdio.h>
#include "reports.h"
#include "models.h"
#include "input.h"

#define PENDING 0
#define COMPLETED 1
#define CANCELLED 2

void systemDashboard()
{
    int totalPersonnel = 0;
    int totalEquipment = 0;
    int availableEquipment = 0;

    int pending = 0;
    int completed = 0;
    int cancelled = 0;

    int totalMemory = 0;
    int totalBurst = 0;
    int pendingCount = 0;

    for (int i = 0; i < unit_count; i++)
    {
        totalPersonnel += units[i].personnel_count;
    }

    for (int i = 0; i < equipment_count; i++)
    {
        totalEquipment += equipment[i].quantity;
        availableEquipment += equipment[i].available;
    }

    for (int i = 0; i < request_count; i++)
    {
        if (requests[i].status == PENDING)
        {
            pending++;
            totalMemory += requests[i].memory_required;
            totalBurst += requests[i].burst_time;
            pendingCount++;
        }
        else if (requests[i].status == COMPLETED)
        {
            completed++;
        }
        else if (requests[i].status == CANCELLED)
        {
            cancelled++;
        }
    }

    printf("\n==============================================\n");
    printf("              MIL-LOG DASHBOARD\n");
    printf("==============================================\n");

    printf("\n--- UNIT SUMMARY ---\n");
    printf("Total Units              : %d\n", unit_count);
    printf("Total Personnel          : %d\n", totalPersonnel);

    printf("\n--- INVENTORY SUMMARY ---\n");
    printf("Equipment Types          : %d\n", equipment_count);
    printf("Total Equipment Units    : %d\n", totalEquipment);
    printf("Available Equipment     : %d\n", availableEquipment);
    printf("Issued Equipment        : %d\n",
           totalEquipment - availableEquipment);

    printf("\n--- REQUEST SUMMARY ---\n");
    printf("Total Requests           : %d\n", request_count);
    printf("Pending Requests         : %d\n", pending);
    printf("Completed Requests       : %d\n", completed);
    printf("Cancelled Requests       : %d\n", cancelled);

    printf("\n--- OS RESOURCE SUMMARY ---\n");
    printf("Pending Memory Required : %d MB\n", totalMemory);

    if (pendingCount > 0)
    {
        printf("Average Burst Time       : %.2f\n",
               (float)totalBurst / pendingCount);
    }
    else
    {
        printf("Average Burst Time       : 0.00\n");
    }

    printf("\n--- SYSTEM STATUS ---\n");
    printf("Unit Management          : READY\n");
    printf("Inventory Management     : READY\n");
    printf("Request Management       : READY\n");
    printf("CPU Scheduling           : READY\n");
    printf("Memory Management        : READY\n");
    printf("Virtual Memory           : READY\n");
    printf("Deadlock Management      : READY\n");
    printf("File Management          : READY\n");

    printf("\n==============================================\n");
}

void requestReport()
{
    int pending = 0;
    int completed = 0;
    int cancelled = 0;

    printf("\n==============================================\n");
    printf("              REQUEST REPORT\n");
    printf("==============================================\n");

    for (int i = 0; i < request_count; i++)
    {
        if (requests[i].status == PENDING)
            pending++;
        else if (requests[i].status == COMPLETED)
            completed++;
        else if (requests[i].status == CANCELLED)
            cancelled++;
    }

    printf("\nTotal Requests     : %d\n", request_count);
    printf("Pending            : %d\n", pending);
    printf("Completed          : %d\n", completed);
    printf("Cancelled          : %d\n", cancelled);

    if (request_count > 0)
    {
        printf("\nCompletion Rate    : %.2f%%\n",
               ((float)completed / request_count) * 100);

        printf("Cancellation Rate  : %.2f%%\n",
               ((float)cancelled / request_count) * 100);
    }

    printf("\n==============================================\n");
}

void inventoryReport()
{
    int total = 0;
    int available = 0;

    printf("\n==============================================\n");
    printf("             INVENTORY REPORT\n");
    printf("==============================================\n");

    for (int i = 0; i < equipment_count; i++)
    {
        total += equipment[i].quantity;
        available += equipment[i].available;

        printf("\nID: %d", equipment[i].id);
        printf("\nName: %s", equipment[i].name);
        printf("\nCategory: %s", equipment[i].category);
        printf("\nTotal Quantity: %d", equipment[i].quantity);
        printf("\nAvailable: %d", equipment[i].available);
        printf("\nIssued: %d\n",
               equipment[i].quantity - equipment[i].available);
    }

    printf("\n----------------------------------------------\n");
    printf("Total Equipment Units : %d\n", total);
    printf("Available Units       : %d\n", available);
    printf("Issued Units          : %d\n", total - available);

    printf("\n==============================================\n");
}

void unitReport()
{
    int totalPersonnel = 0;

    printf("\n==============================================\n");
    printf("                UNIT REPORT\n");
    printf("==============================================\n");

    for (int i = 0; i < unit_count; i++)
    {
        printf("\nUnit ID       : %d\n", units[i].id);
        printf("Unit Name     : %s\n", units[i].name);
        printf("Location      : %s\n", units[i].location);
        printf("Personnel     : %d\n", units[i].personnel_count);

        totalPersonnel += units[i].personnel_count;
    }

    printf("\n----------------------------------------------\n");
    printf("Total Units       : %d\n", unit_count);
    printf("Total Personnel   : %d\n", totalPersonnel);

    printf("\n==============================================\n");
}

void reportsMenu()
{
    int choice;

    do
    {
        printf("\n==============================================\n");
        printf("                REPORTS MODULE\n");
        printf("==============================================\n");

        printf("1. System Dashboard\n");
        printf("2. Request Report\n");
        printf("3. Inventory Report\n");
        printf("4. Unit Report\n");
        printf("0. Back to Main Menu\n");

        choice = readChoice("\nEnter choice: ", 0, 4);

        switch (choice)
        {
            case 1:
                systemDashboard();
                break;

            case 2:
                requestReport();
                break;

            case 3:
                inventoryReport();
                break;

            case 4:
                unitReport();
                break;

            case 0:
                printf("\nReturning to Main Menu...\n");
                break;

            default:
                printf("\nInvalid choice!\n");
        }

    } while (choice != 0);
}