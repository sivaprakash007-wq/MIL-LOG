#include <stdio.h>
#include <string.h>
#include "scheduling.h"
#include "models.h"
#include "requests.h"
#include "input.h"

#define MAX_PROCESSES 200

typedef struct {
    int pid;
    int burst;
    int priority;
    int arrival;
    int completion;
    int turnaround;
    int waiting;
    int response;
} Process;

void printProcessTable(Process p[], int n) {
    double total_waiting = 0;
    double total_turnaround = 0;

    printf("\n");
    printf("================================================================================\n");
    printf("                         SCHEDULING RESULTS                                    \n");
    printf("================================================================================\n");

    printf("%-8s %-8s %-10s %-12s %-12s %-10s %-10s\n",
           "PID", "BURST", "PRIORITY", "COMPLETION",
           "TURNAROUND", "WAITING", "RESPONSE");

    printf("--------------------------------------------------------------------------------\n");

    for (int i = 0; i < n; i++) {
        printf("%-8d %-8d %-10d %-12d %-12d %-10d %-10d\n",
               p[i].pid,
               p[i].burst,
               p[i].priority,
               p[i].completion,
               p[i].turnaround,
               p[i].waiting,
               p[i].response);

        total_waiting += p[i].waiting;
        total_turnaround += p[i].turnaround;
    }

    printf("================================================================================\n");

    printf("\nAverage Waiting Time    : %.2f\n", total_waiting / n);
    printf("Average Turnaround Time : %.2f\n", total_turnaround / n);
}

void printGanttChart(Process p[], int order[], int n) {
    printf("\nGANTT CHART\n\n");

    printf("+");
    for (int i = 0; i < n; i++) {
        printf("--------+");
    }
    printf("\n");

    printf("|");
    for (int i = 0; i < n; i++) {
        printf("  P%-5d|", p[order[i]].pid);
    }
    printf("\n");

    printf("+");
    for (int i = 0; i < n; i++) {
        printf("--------+");
    }
    printf("\n");

    printf("0");

    for (int i = 0; i < n; i++) {
        printf("%8d", p[order[i]].completion);
    }

    printf("\n");
}

int loadPendingProcesses(Process p[]) {
    int n = 0;

    for (int i = 0; i < request_count; i++) {

        if (requests[i].status == 0) {

            p[n].pid = requests[i].id;
            p[n].burst = requests[i].burst_time;
            p[n].priority = requests[i].priority;

            /*
             * Requests are considered to arrive in the
             * order in which they were created.
             */
            p[n].arrival = n;

            p[n].completion = 0;
            p[n].turnaround = 0;
            p[n].waiting = 0;
            p[n].response = 0;

            n++;
        }
    }

    return n;
}

void runFCFS() {
    Process p[MAX_PROCESSES];
    int order[MAX_PROCESSES];

    int n = loadPendingProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    printf("\n========== FIRST COME FIRST SERVE ==========\n");

    int current_time = 0;

    for (int i = 0; i < n; i++) {

        order[i] = i;

        p[i].response = current_time - p[i].arrival;
        p[i].waiting = current_time - p[i].arrival;

        current_time += p[i].burst;

        p[i].completion = current_time;

        p[i].turnaround =
            p[i].completion - p[i].arrival;
    }

    printGanttChart(p, order, n);
    printProcessTable(p, n);
}

void runSJF() {
    Process p[MAX_PROCESSES];
    int order[MAX_PROCESSES];
    int completed[MAX_PROCESSES] = {0};

    int n = loadPendingProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    printf("\n========== SHORTEST JOB FIRST ==========\n");

    int current_time = 0;

    for (int count = 0; count < n; count++) {

        int selected = -1;

        for (int i = 0; i < n; i++) {

            if (!completed[i]) {

                if (p[i].arrival <= current_time) {

                    if (selected == -1 ||
                        p[i].burst < p[selected].burst) {

                        selected = i;
                    }
                }
            }
        }

        /*
         * If no process has arrived yet, select the
         * next process according to arrival order.
         */
        if (selected == -1) {

            for (int i = 0; i < n; i++) {

                if (!completed[i]) {
                    selected = i;
                    break;
                }
            }
        }

        order[count] = selected;

        p[selected].response =
            current_time - p[selected].arrival;

        p[selected].waiting =
            current_time - p[selected].arrival;

        if (p[selected].waiting < 0)
            p[selected].waiting = 0;

        current_time += p[selected].burst;

        p[selected].completion = current_time;

        p[selected].turnaround =
            p[selected].completion - p[selected].arrival;

        completed[selected] = 1;
    }

    printGanttChart(p, order, n);
    printProcessTable(p, n);
}

void runPriority() {
    Process p[MAX_PROCESSES];
    int order[MAX_PROCESSES];
    int completed[MAX_PROCESSES] = {0};

    int n = loadPendingProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    printf("\n========== PRIORITY SCHEDULING ==========\n");
    printf("Priority 1 = Highest Priority\n");

    int current_time = 0;

    for (int count = 0; count < n; count++) {

        int selected = -1;

        for (int i = 0; i < n; i++) {

            if (!completed[i] &&
                p[i].arrival <= current_time) {

                if (selected == -1 ||
                    p[i].priority < p[selected].priority) {

                    selected = i;
                }
            }
        }

        if (selected == -1) {

            for (int i = 0; i < n; i++) {

                if (!completed[i]) {
                    selected = i;
                    break;
                }
            }
        }

        order[count] = selected;

        p[selected].response =
            current_time - p[selected].arrival;

        p[selected].waiting =
            current_time - p[selected].arrival;

        if (p[selected].waiting < 0)
            p[selected].waiting = 0;

        current_time += p[selected].burst;

        p[selected].completion = current_time;

        p[selected].turnaround =
            p[selected].completion - p[selected].arrival;

        completed[selected] = 1;
    }

    printGanttChart(p, order, n);
    printProcessTable(p, n);
}

void runRoundRobin() {
    Process p[MAX_PROCESSES];

    int n = loadPendingProcesses(p);

    if (n == 0) {
        printf("\nNo pending supply requests available.\n");
        return;
    }

    int quantum;

    printf("\n========== ROUND ROBIN ==========\n");

    quantum = readPositiveInt("Enter Time Quantum: ");

    int remaining[MAX_PROCESSES];

    for (int i = 0; i < n; i++) {
        remaining[i] = p[i].burst;
    }

    int current_time = 0;
    int completed_count = 0;

    int first_execution[MAX_PROCESSES];

    for (int i = 0; i < n; i++) {
        first_execution[i] = -1;
    }

    printf("\nGANTT CHART\n\n");

    printf("+");

    for (int i = 0; i < n * 10; i++)
        printf("-");

    printf("+\n");

    while (completed_count < n) {

        int executed = 0;

        for (int i = 0; i < n; i++) {

            if (remaining[i] > 0) {

                executed = 1;

                if (first_execution[i] == -1)
                    first_execution[i] = current_time;

                int execution_time;

                if (remaining[i] > quantum)
                    execution_time = quantum;
                else
                    execution_time = remaining[i];

                printf("| P%d ", p[i].pid);

                current_time += execution_time;

                remaining[i] -= execution_time;

                printf("(%d-%d) ",
                       current_time - execution_time,
                       current_time);

                if (remaining[i] == 0) {

                    p[i].completion = current_time;

                    p[i].turnaround =
                        p[i].completion - p[i].arrival;

                    p[i].waiting =
                        p[i].turnaround - p[i].burst;

                    p[i].response =
                        first_execution[i] - p[i].arrival;

                    completed_count++;
                }
            }
        }

        if (!executed)
            break;
    }

    printf("|\n");

    printProcessTable(p, n);
}

void schedulingMenu() {
    int choice;

    while (1) {

        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|          PROCESS SCHEDULING          |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. FCFS                              |\n");
        printf("| 2. Shortest Job First                |\n");
        printf("| 3. Priority Scheduling               |\n");
        printf("| 4. Round Robin                       |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 4);

        switch (choice) {

            case 1:
                runFCFS();
                break;

            case 2:
                runSJF();
                break;

            case 3:
                runPriority();
                break;

            case 4:
                runRoundRobin();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}