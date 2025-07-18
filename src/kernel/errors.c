#include "errors.h"
#include "../drivers/vga.h"

const char* error_to_string(error_t err) {
    switch(err) {
        case ERR_OK: return "Success";
        case ERR_NOT_FOUND: return "File or directory not found";
        case ERR_NO_SPACE: return "No space left on device";
        case ERR_INVALID_PATH: return "Invalid path";
        case ERR_NOT_DIR: return "Not a directory";
        case ERR_NOT_FILE: return "Not a file";
        case ERR_DIR_NOT_EMPTY: return "Directory not empty";
        case ERR_ALREADY_EXISTS: return "File already exists";
        case ERR_NO_MEMORY: return "Out of memory";
        case ERR_IO_ERROR: return "I/O error";
        case ERR_INVALID_ARGS: return "Invalid arguments";
        default: return "Unknown error";
    }
}

void print_error(const char *prefix, error_t err) {
    if(prefix) {
        prints(prefix);
        prints(": ");
    }
    prints(error_to_string(err));
    prints("\n");
}
