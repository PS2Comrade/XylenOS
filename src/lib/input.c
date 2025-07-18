#include "input.h"
#include "string.h"
#include "../drivers/keyboard.h"
#include "../drivers/vga.h"

int safe_gets(char *buffer, int max_size) {
    int len = 0;
    while(1) {
        char c = get_key();
        
        if(c == '\n') {
            putchar('\n');
            buffer[len] = '\0';
            return len;
        }
        else if(c == '\b') {
            if(len > 0) {
                len--;
                handle_backspace();
            }
        }
        else if(len < max_size - 1 && c >= 32 && c <= 126) {  // Printable chars only
            buffer[len++] = c;
            putchar(c);
        }
    }
}

char *trim_whitespace(char *str) {
    // Trim leading whitespace
    while(*str == ' ' || *str == '\t') str++;
    
    // Trim trailing whitespace
    char *end = str + strlen(str) - 1;
    while(end > str && (*end == ' ' || *end == '\t')) end--;
    *(end + 1) = '\0';
    
    return str;
}

token_list_t tokenize(const char *input) {
    token_list_t result = {0};
    const char *p = input;
    
    while(*p && result.count < MAX_TOKENS) {
        // Skip whitespace
        while(*p == ' ' || *p == '\t') p++;
        if(!*p) break;
        
        // Extract token
        int i = 0;
        while(*p && *p != ' ' && *p != '\t' && i < 63) {
            result.tokens[result.count][i++] = *p++;
        }
        result.tokens[result.count][i] = '\0';
        result.count++;
    }
    
    return result;
}
