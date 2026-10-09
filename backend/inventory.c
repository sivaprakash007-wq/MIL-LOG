#include <stdio.h>
#include <string.h>
#include "inventory.h"
#include "storage.h"
#include "input.h"
void addEquipment() {
    if (equipment_count >= MAX_EQUIPMENT) {
        printf("\nMaximum equipment capacity reached.\n");
        return;
    }

    Equipment e;

    printf("\n========== ADD EQUIPMENT ==========\n");

    e.id = readPositiveInt("Enter Equipment ID: ");

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == e.id) {
            printf("\nEquipment ID already exists.\n");
            return;
        }
    }

    readString("Enter Equipment Name: ",
           e.name,
           sizeof(e.name));

    readString("Enter Category: ",
           e.category,
           sizeof(e.category));

    e.quantity =
    readPositiveInt("Enter Quantity: ");

    if (e.quantity < 0) {
        printf("\nInvalid quantity.\n");
        return;
    }

    e.available = e.quantity;

    equipment[equipment_count] = e;
    equipment_count++;

    saveInventory();

    printf("\nEquipment added successfully.\n");
}

void displayInventory() {
    if (equipment_count == 0) {
        printf("\nInventory is empty.\n");
        return;
    }

    printf("\n============================== EQUIPMENT INVENTORY ==============================\n");

    printf("%-10s %-25s %-20s %-12s %-12s\n",
           "ID", "EQUIPMENT", "CATEGORY", "QUANTITY", "AVAILABLE");

    printf("---------------------------------------------------------------------------------\n");

    for (int i = 0; i < equipment_count; i++) {
        printf("%-10d %-25s %-20s %-12d %-12d\n",
               equipment[i].id,
               equipment[i].name,
               equipment[i].category,
               equipment[i].quantity,
               equipment[i].available);
    }

    printf("=================================================================================\n");
}

void searchEquipment() {
    int id;

    id = readPositiveInt("\nEnter Equipment ID to search: ");

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == id) {
            printf("\n========== EQUIPMENT FOUND ==========\n");
            printf("Equipment ID : %d\n", equipment[i].id);
            printf("Name         : %s\n", equipment[i].name);
            printf("Category     : %s\n", equipment[i].category);
            printf("Total        : %d\n", equipment[i].quantity);
            printf("Available    : %d\n", equipment[i].available);
            return;
        }
    }

    printf("\nEquipment not found.\n");
}

void issueEquipment() {
    int id;
    int quantity;

    id = readPositiveInt("Enter Equipment ID: ");

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == id) {

            quantity = readPositiveInt("Enter Quantity to Issue: ");

            if (quantity > equipment[i].available) {
                printf("\nInsufficient equipment available.\n");
                printf("Available quantity: %d\n", equipment[i].available);
                return;
            }

            equipment[i].available -= quantity;

            saveInventory();

            printf("\nEquipment issued successfully.\n");
            printf("Remaining available: %d\n", equipment[i].available);

            return;
        }
    }

    printf("\nEquipment not found.\n");
}

void returnEquipment() {
    int id;
    int quantity;

    id = readPositiveInt("Enter Equipment ID: ");

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == id) {

            quantity = readPositiveInt("Enter Quantity to Return: ");

            if (equipment[i].available + quantity > equipment[i].quantity) {
                printf("\nReturn quantity exceeds issued quantity.\n");
                return;
            }

            equipment[i].available += quantity;

            saveInventory();

            printf("\nEquipment returned successfully.\n");
            printf("Available quantity: %d\n", equipment[i].available);

            return;
        }
    }

    printf("\nEquipment not found.\n");
}

void deleteEquipment() {
    int id;

    id = readPositiveInt("\nEnter Equipment ID to delete: ");

    for (int i = 0; i < equipment_count; i++) {
        if (equipment[i].id == id) {

            for (int j = i; j < equipment_count - 1; j++) {
                equipment[j] = equipment[j + 1];
            }

            equipment_count--;

            saveInventory();

            printf("\nEquipment deleted successfully.\n");
            return;
        }
    }

    printf("\nEquipment not found.\n");
}

void saveInventory() {
    FILE *file = openDataFile("inventory.dat", "wb");

    if (file == NULL) {
        printf("\nUnable to save inventory data.\n");
        return;
    }

    fwrite(&equipment_count, sizeof(int), 1, file);
    fwrite(equipment, sizeof(Equipment), equipment_count, file);

    fclose(file);
}

void loadInventory() {
    FILE *file = openDataFile("inventory.dat", "rb");

    if (file == NULL) {
        return;
    }

    if (fread(&equipment_count, sizeof(int), 1, file) != 1) {
        equipment_count = 0;
        fclose(file);
        return;
    }

    if (equipment_count < 0 || equipment_count > MAX_EQUIPMENT) {
        equipment_count = 0;
        fclose(file);
        return;
    }

    if (fread(equipment, sizeof(Equipment), equipment_count, file) != (size_t)equipment_count)
        equipment_count = 0;

    fclose(file);
}

void inventoryMenu() {
    int choice;

    while (1) {
        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|       EQUIPMENT & INVENTORY          |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. Add Equipment                     |\n");
        printf("| 2. Display Inventory                 |\n");
        printf("| 3. Search Equipment                  |\n");
        printf("| 4. Issue Equipment                   |\n");
        printf("| 5. Return Equipment                  |\n");
        printf("| 6. Delete Equipment                  |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 6);

        switch (choice) {
            case 1:
                addEquipment();
                break;

            case 2:
                displayInventory();
                break;

            case 3:
                searchEquipment();
                break;

            case 4:
                issueEquipment();
                break;

            case 5:
                returnEquipment();
                break;

            case 6:
                deleteEquipment();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}