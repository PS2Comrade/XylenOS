#ifndef ERRORS_H
#define ERRORS_H

typedef enum {
    ERR_OK = 0,
    ERR_NOT_FOUND,
    ERR_NO_SPACE,
    ERR_INVALID_PATH,
    ERR_NOT_DIR,
    ERR_NOT_FILE,
    ERR_DIR_NOT_EMPTY,
    ERR_ALREADY_EXISTS,
    ERR_NO_MEMORY,
    ERR_IO_ERROR,
    ERR_INVALID_ARGS
} error_t;

const char* error_to_string(error_t err);
void print_error(const char *prefix, error_t err);

#endif
