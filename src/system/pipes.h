#ifndef PIPES_H
#define PIPES_H

#include "../kernel/types.h"

#define PIPE_BUFFER_SIZE 1024

typedef struct {
    char buffer[PIPE_BUFFER_SIZE];
    int write_pos;
    int read_pos;
    int is_active;
} pipe_t;

pipe_t* pipe_create(void);
void pipe_destroy(pipe_t *pipe);
int pipe_write(pipe_t *pipe, const char *data, int len);
int pipe_read(pipe_t *pipe, char *buffer, int max_len);
int pipe_has_data(pipe_t *pipe);

#endif
