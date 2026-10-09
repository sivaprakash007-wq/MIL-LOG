#include <stdio.h>
#include "paging.h"
#include "models.h"
#include "input.h"

#define MAX_REFERENCE 200
#define MAX_FRAMES 20

void displayFrames(int frames[], int frame_count) {
    printf("Frames: ");

    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == -1)
            printf("[ - ] ");
        else
            printf("[ %d ] ", frames[i]);
    }

    printf("\n");
}

int findPage(int frames[], int frame_count, int page) {
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == page)
            return i;
    }

    return -1;
}

void getReferenceString(int reference[], int *length) {

    printf("\n========== PAGE REFERENCE CONFIGURATION ==========\n");

    *length = readChoice("Enter number of page references: ", 1, MAX_REFERENCE);

    printf("\nEnter page references.\n");
    printf("Use existing Supply Request IDs where possible.\n\n");

    for (int i = 0; i < *length; i++) {

        char prompt[80];
        snprintf(prompt, sizeof(prompt), "Reference %d: ", i + 1);
        reference[i] = readNonNegativeInt(prompt);
    }
}

int getFrameCount() {

    return readChoice("\nEnter number of page frames: ", 1, MAX_FRAMES);
}

void displayPagingSummary(
    int hits,
    int faults,
    int total
) {

    double hit_ratio =
        ((double) hits / total) * 100.0;

    double fault_ratio =
        ((double) faults / total) * 100.0;

    printf("\n========================================\n");
    printf("          PAGING SUMMARY\n");
    printf("========================================\n");

    printf("Total References : %d\n", total);
    printf("Page Hits        : %d\n", hits);
    printf("Page Faults      : %d\n", faults);
    printf("Hit Ratio        : %.2f%%\n", hit_ratio);
    printf("Fault Ratio      : %.2f%%\n", fault_ratio);

    printf("========================================\n");
}

void runFIFO() {

    int reference[MAX_REFERENCE];
    int length;

    getReferenceString(reference, &length);

    if (length == 0)
        return;

    int frame_count = getFrameCount();

    if (frame_count == 0)
        return;

    int frames[MAX_FRAMES];

    for (int i = 0; i < frame_count; i++)
        frames[i] = -1;

    int pointer = 0;
    int hits = 0;
    int faults = 0;

    printf("\n========== FIFO PAGE REPLACEMENT ==========\n\n");

    for (int i = 0; i < length; i++) {

        int page = reference[i];

        int position =
            findPage(frames, frame_count, page);

        if (position != -1) {

            hits++;

            printf("Reference %d -> PAGE HIT  -> ", page);
            displayFrames(frames, frame_count);
        }
        else {

            faults++;

            frames[pointer] = page;

            pointer =
                (pointer + 1) % frame_count;

            printf("Reference %d -> PAGE FAULT -> ",
                   page);

            displayFrames(frames, frame_count);
        }
    }

    displayPagingSummary(
        hits,
        faults,
        length
    );
}

void runLRU() {

    int reference[MAX_REFERENCE];
    int length;

    getReferenceString(reference, &length);

    if (length == 0)
        return;

    int frame_count = getFrameCount();

    if (frame_count == 0)
        return;

    int frames[MAX_FRAMES];
    int last_used[MAX_FRAMES];

    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    int hits = 0;
    int faults = 0;

    printf("\n========== LRU PAGE REPLACEMENT ==========\n\n");

    for (int i = 0; i < length; i++) {

        int page = reference[i];

        int position =
            findPage(frames, frame_count, page);

        if (position != -1) {

            hits++;

            last_used[position] = i;

            printf("Reference %d -> PAGE HIT  -> ",
                   page);

            displayFrames(frames, frame_count);
        }
        else {

            faults++;

            int replace_index = -1;

            /*
             * First check for an empty frame.
             */
            for (int j = 0; j < frame_count; j++) {

                if (frames[j] == -1) {
                    replace_index = j;
                    break;
                }
            }

            /*
             * If no empty frame exists,
             * replace the least recently used page.
             */
            if (replace_index == -1) {

                replace_index = 0;

                for (int j = 1; j < frame_count; j++) {

                    if (last_used[j] <
                        last_used[replace_index]) {

                        replace_index = j;
                    }
                }
            }

            frames[replace_index] = page;
            last_used[replace_index] = i;

            printf("Reference %d -> PAGE FAULT -> ",
                   page);

            displayFrames(frames, frame_count);
        }
    }

    displayPagingSummary(
        hits,
        faults,
        length
    );
}

int findOptimalReplacement(
    int frames[],
    int frame_count,
    int reference[],
    int length,
    int current
) {

    int farthest = current;
    int replacement = -1;

    for (int i = 0; i < frame_count; i++) {

        int j;

        for (j = current + 1;
             j < length;
             j++) {

            if (frames[i] == reference[j])
                break;
        }

        /*
         * Page is never used again.
         */
        if (j == length)
            return i;

        /*
         * Find the page whose next use is
         * farthest in the future.
         */
        if (j > farthest) {

            farthest = j;
            replacement = i;
        }
    }

    if (replacement == -1)
        replacement = 0;

    return replacement;
}

void runOptimal() {

    int reference[MAX_REFERENCE];
    int length;

    getReferenceString(reference, &length);

    if (length == 0)
        return;

    int frame_count = getFrameCount();

    if (frame_count == 0)
        return;

    int frames[MAX_FRAMES];

    for (int i = 0; i < frame_count; i++)
        frames[i] = -1;

    int hits = 0;
    int faults = 0;

    printf("\n========== OPTIMAL PAGE REPLACEMENT ==========\n\n");

    for (int i = 0; i < length; i++) {

        int page = reference[i];

        int position =
            findPage(frames, frame_count, page);

        if (position != -1) {

            hits++;

            printf("Reference %d -> PAGE HIT  -> ",
                   page);

            displayFrames(frames, frame_count);
        }
        else {

            faults++;

            int replace_index = -1;

            /*
             * First use an empty frame if available.
             */
            for (int j = 0; j < frame_count; j++) {

                if (frames[j] == -1) {
                    replace_index = j;
                    break;
                }
            }

            /*
             * If all frames are occupied,
             * use the Optimal replacement strategy.
             */
            if (replace_index == -1) {

                replace_index =
                    findOptimalReplacement(
                        frames,
                        frame_count,
                        reference,
                        length,
                        i
                    );
            }

            frames[replace_index] = page;

            printf("Reference %d -> PAGE FAULT -> ",
                   page);

            displayFrames(frames, frame_count);
        }
    }

    displayPagingSummary(
        hits,
        faults,
        length
    );
}

void pagingMenu() {

    int choice;

    while (1) {

        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|       VIRTUAL MEMORY / PAGING        |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. FIFO Page Replacement             |\n");
        printf("| 2. LRU Page Replacement              |\n");
        printf("| 3. Optimal Page Replacement          |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 3);

        switch (choice) {

            case 1:
                runFIFO();
                break;

            case 2:
                runLRU();
                break;

            case 3:
                runOptimal();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}