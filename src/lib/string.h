#ifndef STRING_H
#define STRING_H

#include "../kernel/types.h"

char *strncat(char *dest, const char *src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, int n);
int strlen(const char *s);
char *strncpy(char *d, const char *s, int n);

#endif
