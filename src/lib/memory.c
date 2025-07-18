#include "memory.h"
#include <stdint.h>

void *memcpy(void *d, const void *s, int n) {
    char *dd = d; const char *ss = s; for (int i = 0; i < n; i++) dd[i] = ss[i]; return d;
}

void *memset(void *dst, int val, int n) {
    unsigned char *d = (unsigned char*)dst;
    for(int i=0;i<n;i++) d[i] = (unsigned char)val;
    return dst;
}
