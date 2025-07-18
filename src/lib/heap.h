#ifndef HEAP_H
#define HEAP_H

#include "../kernel/types.h"

#define HEAP_SIZE (128 * 1024)  // 128KB heap

typedef struct heap_block {
    size_t size;
    int is_free;
    struct heap_block *next;
} heap_block_t;

void heap_init(void);
void* kmalloc(size_t size);
void kfree(void *ptr);
void heap_dump(void);  // For debugging

#endif
