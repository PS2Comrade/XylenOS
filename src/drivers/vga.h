#ifndef VGA_H
#define VGA_H

#include <stdint.h>

void clear_screen(void);
void putchar(char c);
void prints(const char *s);
void enable_cursor(void);
void handle_backspace(void);  // Add this!

extern int cursor_x, cursor_y;  // Make these available to other files

#endif
