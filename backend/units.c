#include <stdio.h>
#include <string.h>
#include "units.h"
#include "storage.h"
#include "input.h"
void addUnit() {
    if (unit_count >= MAX_UNITS) {
        printf("\nMaximum number of units reached.\n");
        return;
    }

    Unit u;

    printf("\n========== ADD MILITARY UNIT ==========\n");

    u.id = readPositiveInt("Enter Unit ID: ");

    for (int i = 0; i < unit_count; i++) {
        if (units[i].id == u.id) {
            printf("\nUnit ID already exists.\n");
            return;
        }
    }

    readString("Enter Unit Name: ", u.name, sizeof(u.name));

    readString("Enter Location: ", u.location, sizeof(u.location));

    u.personnel_count =
    readNonNegativeInt("Enter Personnel Count: ");

    units[unit_count] = u;
    unit_count++;

    saveUnits();

    printf("\nUnit added successfully.\n");
}

void displayUnits() {
    if (unit_count == 0) {
        printf("\nNo units registered.\n");
        return;
    }

    printf("\n===================== UNIT LIST =====================\n");

    printf("%-10s %-25s %-20s %-12s\n",
           "ID", "UNIT NAME", "LOCATION", "PERSONNEL");

    printf("------------------------------------------------------\n");

    for (int i = 0; i < unit_count; i++) {
        printf("%-10d %-25s %-20s %-12d\n",
               units[i].id,
               units[i].name,
               units[i].location,
               units[i].personnel_count);
    }

    printf("======================================================\n");
}

void searchUnit() {
    int id;

    id = readPositiveInt("Enter Unit ID: ");

    for (int i = 0; i < unit_count; i++) {
        if (units[i].id == id) {
            printf("\n========== UNIT FOUND ==========\n");
            printf("Unit ID        : %d\n", units[i].id);
            printf("Unit Name      : %s\n", units[i].name);
            printf("Location       : %s\n", units[i].location);
            printf("Personnel      : %d\n", units[i].personnel_count);
            return;
        }
    }

    printf("\nUnit not found.\n");
}

void deleteUnit() {
    int id;

    id = readPositiveInt("Enter Unit ID: ");

    for (int i = 0; i < unit_count; i++) {
        if (units[i].id == id) {

            for (int j = i; j < unit_count - 1; j++) {
                units[j] = units[j + 1];
            }

            unit_count--;

            saveUnits();

            printf("\nUnit deleted successfully.\n");
            return;
        }
    }

    printf("\nUnit not found.\n");
}

void saveUnits() {
    FILE *file = openDataFile("units.dat", "wb");

    if (file == NULL) {
        printf("\nUnable to save unit data.\n");
        return;
    }

    fwrite(&unit_count, sizeof(int), 1, file);
    fwrite(units, sizeof(Unit), unit_count, file);

    fclose(file);
}

void loadUnits() {
    FILE *file = openDataFile("units.dat", "rb");

    if (file == NULL) {
        return;
    }

    if (fread(&unit_count, sizeof(int), 1, file) != 1) {
        unit_count = 0;
        fclose(file);
        return;
    }

    if (unit_count < 0 || unit_count > MAX_UNITS) {
        unit_count = 0;
        fclose(file);
        return;
    }

    if (fread(units, sizeof(Unit), unit_count, file) != (size_t)unit_count)
        unit_count = 0;

    fclose(file);
}

void unitMenu() {
    int choice;

    while (1) {
        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|          UNIT MANAGEMENT             |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. Add Unit                          |\n");
        printf("| 2. Display All Units                 |\n");
        printf("| 3. Search Unit                       |\n");
        printf("| 4. Delete Unit                       |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 4);

        switch (choice) {
            case 1:
                addUnit();
                break;

            case 2:
                displayUnits();
                break;

            case 3:
                searchUnit();
                break;

            case 4:
                deleteUnit();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}