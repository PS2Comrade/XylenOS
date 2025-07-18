#ifndef MEMORY_H
#define MEMORY_H

#include "../kernel/types.h"

void *memcpy(void *d, const void *s, int n);
void *memset(void *dst, int val, int n);

#endif
