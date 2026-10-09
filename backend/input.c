#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include "input.h"

int readInt(const char *prompt)
{
    char buffer[100];
    char *end;
    long value;

    while (1)
    {
        printf("%s", prompt);

        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            exit(0);

        errno = 0;
        value = strtol(buffer, &end, 10);

        while (*end == ' ' || *end == '\t')
            end++;

        if (end != buffer && (*end == '\n' || *end == '\0') &&
            errno != ERANGE && value >= INT_MIN && value <= INT_MAX)
        {
            return (int)value;
        }

        printf("Invalid input. Please enter a valid integer.\n");
    }
}

int readPositiveInt(const char *prompt)
{
    int value;

    while (1)
    {
        value = readInt(prompt);

        if (value > 0)
            return value;

        printf("Please enter a value greater than 0.\n");
    }
}

int readNonNegativeInt(const char *prompt)
{
    int value;

    while (1)
    {
        value = readInt(prompt);

        if (value >= 0)
            return value;

        printf("Please enter 0 or a positive value.\n");
    }
}

int readChoice(const char *prompt, int min, int max)
{
    int value;

    while (1)
    {
        value = readInt(prompt);

        if (value >= min && value <= max)
            return value;

        printf("Invalid choice. Enter a value between %d and %d.\n",
               min, max);
    }
}

void readString(const char *prompt, char *buffer, int size)
{
    while (1)
    {
        int ch;
        size_t length;

        printf("%s", prompt);

        if (fgets(buffer, size, stdin) == NULL)
            exit(0);

        length = strcspn(buffer, "\n");

        if (buffer[length] == '\n')
        {
            buffer[length] = '\0';
        }
        else
        {
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;

            printf("Input is too long. Maximum %d characters allowed.\n",
                   size - 1);
            continue;
        }

        if (strlen(buffer) > 0)
            return;

        printf("Input cannot be empty.\n");
    }
}
