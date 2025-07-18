#ifndef INPUT_H
#define INPUT_H

#include "../kernel/types.h"

#define MAX_INPUT_LINE 256
#define MAX_TOKENS 16

typedef struct {
    char tokens[MAX_TOKENS][64];
    int count;
} token_list_t;

int safe_gets(char *buffer, int max_size);
token_list_t tokenize(const char *input);
char *trim_whitespace(char *str);

#endif
