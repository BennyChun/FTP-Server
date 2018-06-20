#ifndef _UTILITY__

#define _UTILITY__

int strcicmp(char const *a, char const *b);
char* substring(char *string, int pos, int len);
void strreverse(char* begin, char* end);
void itoa(int value, char* str, int base);

#endif