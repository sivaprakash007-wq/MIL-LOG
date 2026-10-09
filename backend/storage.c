#include <stdio.h>
#include <string.h>
#include "storage.h"

FILE *openDataFile(const char *filename, const char *mode)
{
    char path[256];
    FILE *file;

    snprintf(path, sizeof(path), "data/%s", filename);
    file = fopen(path, mode);

    if (file != NULL)
        return file;

    snprintf(path, sizeof(path), "../data/%s", filename);
    file = fopen(path, mode);

    return file;
}