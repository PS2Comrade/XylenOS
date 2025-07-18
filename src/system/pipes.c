#include "pipes.h"
#include "../lib/heap.h"
#include "../lib/memory.h"
#include "../lib/string.h"

pipe_t* pipe_create(void) {
    pipe_t *pipe = (pipe_t*)kmalloc(sizeof(pipe_t));
    if(!pipe) return NULL;
    
    pipe->write_pos = 0;
    pipe->read_pos = 0;
    pipe->is_active = 1;
    
    return pipe;
}

void pipe_destroy(pipe_t *pipe) {
    if(pipe) {
        pipe->is_active = 0;
        kfree(pipe);
    }
}

int pipe_write(pipe_t *pipe, const char *data, int len) {
    if(!pipe || !pipe->is_active) return 0;
    
    int written = 0;
    for(int i = 0; i < len; i++) {
        if((pipe->write_pos + 1) % PIPE_BUFFER_SIZE == pipe->read_pos) {
            break;  // Buffer full
        }
        
        pipe->buffer[pipe->write_pos] = data[i];
        pipe->write_pos = (pipe->write_pos + 1) % PIPE_BUFFER_SIZE;
        written++;
    }
    
    return written;
}

int pipe_read(pipe_t *pipe, char *buffer, int max_len) {
    if(!pipe || !pipe->is_active) return 0;
    
    int read = 0;
    while(read < max_len && pipe->read_pos != pipe->write_pos) {
        buffer[read] = pipe->buffer[pipe->read_pos];
        pipe->read_pos = (pipe->read_pos + 1) % PIPE_BUFFER_SIZE;
        read++;
    }
    
    return read;
}

int pipe_has_data(pipe_t *pipe) {
    if(!pipe || !pipe->is_active) return 0;
    return pipe->read_pos != pipe->write_pos;
}
