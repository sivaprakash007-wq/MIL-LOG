#ifndef INPUT_H
#define INPUT_H

int readInt(const char *prompt);
int readPositiveInt(const char *prompt);
int readNonNegativeInt(const char *prompt);
int readChoice(const char *prompt, int min, int max);
void readString(const char *prompt, char *buffer, int size);

#endif