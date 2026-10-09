#include <stdio.h>
#include <string.h>
#include "input.h"
#include "filesystem.h"
#include "storage.h"

#define MAX_FILES 50
#define MAX_BLOCKS 100

typedef struct {
    int id;
    char name[50];
    int size;
} FileRecord;

FileRecord files[MAX_FILES];
int file_count = 0;


/* ================= FILE PERSISTENCE ================= */

void saveFiles()
{
    FILE *file = openDataFile("files.dat", "wb");

    if (file == NULL)
    {
        printf("\nError: Unable to save file records.\n");
        return;
    }

    fwrite(&file_count, sizeof(int), 1, file);
    fwrite(files, sizeof(FileRecord), file_count, file);

    fclose(file);
}


void loadFiles()
{
    FILE *file = openDataFile("files.dat", "rb");

    if (file == NULL)
    {
        file_count = 0;
        return;
    }

    if (fread(&file_count, sizeof(int), 1, file) != 1)
    {
        file_count = 0;
        fclose(file);
        return;
    }

    if (file_count < 0 || file_count > MAX_FILES)
    {
        file_count = 0;
        fclose(file);
        return;
    }

    if (fread(files, sizeof(FileRecord), file_count, file) !=
        (size_t)file_count)
    {
        file_count = 0;
    }

    fclose(file);
}


/* ================= CREATE FILE ================= */

void createFile()
{
    if (file_count >= MAX_FILES)
    {
        printf("\nMaximum file limit reached.\n");
        return;
    }

    FileRecord f;

    printf("\n========== CREATE FILE ==========\n");

    f.id = readPositiveInt("Enter File ID: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == f.id)
        {
            printf("\nFile ID already exists.\n");
            return;
        }
    }

    readString("Enter File Name: ", f.name, sizeof(f.name));

    f.size = readPositiveInt("Enter File Size (number of blocks): ");;

    if (f.size <= 0 || f.size > MAX_BLOCKS)
    {
        printf("\nInvalid file size.\n");
        return;
    }

    files[file_count] = f;
    file_count++;

    saveFiles();

    printf("\nFile created successfully.\n");
}


/* ================= DISPLAY FILES ================= */

void displayFiles()
{
    if (file_count == 0)
    {
        printf("\nNo files available.\n");
        return;
    }

    printf("\n");
    printf("============================================================\n");
    printf("                    FILE DIRECTORY\n");
    printf("============================================================\n");

    printf("%-10s %-35s %-10s\n",
           "ID",
           "FILE NAME",
           "BLOCKS");

    printf("------------------------------------------------------------\n");

    for (int i = 0; i < file_count; i++)
    {
        printf("%-10d %-35s %-10d\n",
               files[i].id,
               files[i].name,
               files[i].size);
    }

    printf("============================================================\n");
}


/* ================= SEARCH FILE ================= */

void searchFile()
{
    int id = readPositiveInt("\nEnter File ID to search: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == id)
        {
            printf("\n========== FILE FOUND ==========\n");

            printf("File ID   : %d\n", files[i].id);
            printf("File Name : %s\n", files[i].name);
            printf("Size      : %d blocks\n", files[i].size);

            return;
        }
    }

    printf("\nFile not found.\n");
}


/* ================= DELETE FILE ================= */

void deleteFile()
{
    int id = readPositiveInt("\nEnter File ID to delete: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == id)
        {
            for (int j = i; j < file_count - 1; j++)
            {
                files[j] = files[j + 1];
            }

            file_count--;

            saveFiles();

            printf("\nFile deleted successfully.\n");

            return;
        }
    }

    printf("\nFile not found.\n");
}


/* ================= CONTIGUOUS ALLOCATION ================= */

void contiguousAllocation()
{
    int id;
    int start_block;

    printf("\n========== CONTIGUOUS ALLOCATION ==========\n");

    id = readPositiveInt("Enter File ID: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == id)
        {
            start_block = readNonNegativeInt("Enter starting block: ");

            if (start_block < 0 ||
                start_block + files[i].size > MAX_BLOCKS)
            {
                printf("\nInvalid block range.\n");
                return;
            }

            printf("\nFile: %s\n", files[i].name);
            printf("Allocation method: CONTIGUOUS\n");
            printf("Starting Block: %d\n", start_block);
            printf("Number of Blocks: %d\n",
                   files[i].size);

            printf("\nAllocated Blocks:\n");

            for (int j = 0; j < files[i].size; j++)
            {
                printf("%d", start_block + j);

                if (j != files[i].size - 1)
                    printf(" -> ");
            }

            printf("\n");

            return;
        }
    }

    printf("\nFile not found.\n");
}


/* ================= LINKED ALLOCATION ================= */

void linkedAllocation()
{
    int id;
    int start_block;

    printf("\n========== LINKED ALLOCATION ==========\n");

    id = readPositiveInt("Enter File ID: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == id)
        {
            start_block = readNonNegativeInt("Enter starting block: ");

            if (start_block < 0 ||
                start_block >= MAX_BLOCKS)
            {
                printf("\nInvalid starting block.\n");
                return;
            }

            printf("\nFile: %s\n", files[i].name);
            printf("Allocation method: LINKED\n");

            printf("\nLinked Blocks:\n");

            int current = start_block;

            for (int j = 0; j < files[i].size; j++)
            {
                printf("%d", current);

                if (j != files[i].size - 1)
                {
                    current = (current + 7) % MAX_BLOCKS;

                    printf(" -> ");
                }
            }

            printf(" -> NULL\n");

            return;
        }
    }

    printf("\nFile not found.\n");
}


/* ================= INDEXED ALLOCATION ================= */

void indexedAllocation()
{
    int id;
    int index_block;

    printf("\n========== INDEXED ALLOCATION ==========\n");

    id = readPositiveInt("Enter File ID: ");

    for (int i = 0; i < file_count; i++)
    {
        if (files[i].id == id)
        {
            index_block = readNonNegativeInt("Enter index block: ");

            if (index_block < 0 ||
                index_block >= MAX_BLOCKS)
            {
                printf("\nInvalid index block.\n");
                return;
            }

            printf("\nFile: %s\n", files[i].name);
            printf("Allocation method: INDEXED\n");
            printf("Index Block: %d\n", index_block);

            printf("\nIndex Block Contents:\n");

            for (int j = 0; j < files[i].size; j++)
            {
                int data_block =
                    (index_block + j + 1) % MAX_BLOCKS;

                printf("Index[%d] -> Block %d\n",
                       j,
                       data_block);
            }

            return;
        }
    }

    printf("\nFile not found.\n");
}


/* ================= FILE SYSTEM MENU ================= */

void fileSystemMenu()
{
    int choice;

    while (1)
    {
        printf("\n");
        printf("+--------------------------------------+\n");
        printf("|          FILE MANAGEMENT             |\n");
        printf("+--------------------------------------+\n");
        printf("| 1. Create File                       |\n");
        printf("| 2. Display Files                     |\n");
        printf("| 3. Search File                       |\n");
        printf("| 4. Delete File                       |\n");
        printf("| 5. Contiguous Allocation             |\n");
        printf("| 6. Linked Allocation                 |\n");
        printf("| 7. Indexed Allocation                |\n");
        printf("| 0. Back to Main Menu                 |\n");
        printf("+--------------------------------------+\n");

        choice = readChoice("Enter choice: ", 0, 7);

        switch (choice)
        {
            case 1:
                createFile();
                break;

            case 2:
                displayFiles();
                break;

            case 3:
                searchFile();
                break;

            case 4:
                deleteFile();
                break;

            case 5:
                contiguousAllocation();
                break;

            case 6:
                linkedAllocation();
                break;

            case 7:
                indexedAllocation();
                break;

            case 0:
                return;

            default:
                printf("\nInvalid choice.\n");
        }
    }
}