#include "string.h"
#include <stdint.h>

char *strncat(char *dest, const char *src, size_t n) {
    char *d = dest;
    while (*d) d++;
    while (n-- && *src) *d++ = *src++;
    *d = 0;
    return dest;
}

char *strcpy(char *dst, const char *src) {
    char *ret = dst;
    while ((*dst++ = *src++));
    return ret;
}

int strcmp(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return (uint8_t)*a - (uint8_t)*b;
}

int strncmp(const char *a, const char *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] != b[i] || !a[i] || !b[i]) return (uint8_t)a[i] - (uint8_t)*b;
    }
    return 0;
}

int strlen(const char *s) {
    int l = 0; while (s[l]) l++; return l;
}

char *strncpy(char *d, const char *s, int n) {
    int i = 0; for (; i < n && s[i]; i++) d[i] = s[i];
    for (; i < n; i++) d[i] = 0; return d;
}
