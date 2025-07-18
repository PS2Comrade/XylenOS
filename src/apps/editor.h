#ifndef EDITOR_H
#define EDITOR_H

#include "../kernel/types.h"

#define EDITOR_MAX_LINES 50
#define EDITOR_MAX_LINE_LEN 80

typedef struct {
    char lines[EDITOR_MAX_LINES][EDITOR_MAX_LINE_LEN];
    int line_count;
    int cursor_line;
    int cursor_col;
    char filename[64];
} editor_state_t;

void editor_run(const char *filename);

#endif
