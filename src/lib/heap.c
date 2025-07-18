#include "heap.h"
#include "memory.h"
#include "../drivers/vga.h"

static char heap_memory[HEAP_SIZE];
static heap_block_t *heap_start = NULL;
static int heap_initialized = 0;

void heap_init(void) {
    heap_start = (heap_block_t*)heap_memory;
    heap_start->size = HEAP_SIZE - sizeof(heap_block_t);
    heap_start->is_free = 1;
    heap_start->next = NULL;
    heap_initialized = 1;
}

void* kmalloc(size_t size) {
    if(!heap_initialized) heap_init();
    
    // Align to 4 bytes
    size = (size + 3) & ~3;
    
    heap_block_t *current = heap_start;
    
    while(current) {
        if(current->is_free && current->size >= size) {
            // Split block if it's much larger
            if(current->size > size + sizeof(heap_block_t) + 16) {
                heap_block_t *new_block = (heap_block_t*)((char*)current + sizeof(heap_block_t) + size);
                new_block->size = current->size - size - sizeof(heap_block_t);
                new_block->is_free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            
            current->is_free = 0;
            return (char*)current + sizeof(heap_block_t);
        }
        current = current->next;
    }
    
    return NULL;  // Out of memory
}

void kfree(void *ptr) {
    if(!ptr) return;
    
    heap_block_t *block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));
    block->is_free = 1;
    
    // Coalesce with next block if it's free
    if(block->next && block->next->is_free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
    }
    
    // Coalesce with previous block (simple linear search)
    heap_block_t *prev = heap_start;
    while(prev && prev->next != block) prev = prev->next;
    
    if(prev && prev->is_free) {
        prev->size += sizeof(heap_block_t) + block->size;
        prev->next = block->next;
    }
}

void heap_dump(void) {
    prints("=== HEAP DUMP ===\n");
    heap_block_t *current = heap_start;
    int total_free = 0, total_used = 0;
    
    while(current) {
        prints(current->is_free ? "FREE: " : "USED: ");
        // Simple number printing
        char size_str[16];
        int size = current->size;
        int i = 0;
        do {
            size_str[i++] = '0' + (size % 10);
            size /= 10;
        } while(size > 0);
        
        for(int j = i-1; j >= 0; j--) putchar(size_str[j]);
        prints(" bytes\n");
        
        if(current->is_free) total_free += current->size;
        else total_used += current->size;
        
        current = current->next;
    }
    prints("================\n");
}
