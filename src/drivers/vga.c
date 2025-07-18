#include "vga.h"

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define VGA_MEM ((uint16_t*)0xB8000)

int cursor_x = 0, cursor_y = 0;  // Remove static so other files can see these
static const uint8_t VGA_COLOR = 0x0F;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

static void move_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 0x0F); outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E); outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

void enable_cursor() {
    outb(0x3D4, 0x0A); outb(0x3D5, inb(0x3D5) & 0xC0);
    outb(0x3D4, 0x0B); outb(0x3D5, 0x0F);
}

static void scroll() {
    if (cursor_y < VGA_HEIGHT) return;
    for (int y = 1; y < VGA_HEIGHT; y++)
        for (int x = 0; x < VGA_WIDTH; x++)
            VGA_MEM[(y-1)*VGA_WIDTH + x] = VGA_MEM[y*VGA_WIDTH + x];
    for (int x = 0; x < VGA_WIDTH; x++)
        VGA_MEM[(VGA_HEIGHT-1)*VGA_WIDTH + x] = (uint16_t)(' ' | (VGA_COLOR << 8));
    cursor_y = VGA_HEIGHT - 1;
}

// NEW: Proper backspace handling function
void handle_backspace() {
    if (cursor_x > 0) {
        cursor_x--;
        VGA_MEM[cursor_y * VGA_WIDTH + cursor_x] = (uint16_t)(' ' | (VGA_COLOR << 8));
        move_cursor();
    }
}

void putchar(char c) {
    if (c == '\r') return;
    if (c == '\b') {  // Handle backspace here too
        handle_backspace();
        return;
    }
    if (c == '\n') {
        cursor_x = 0; cursor_y++;
        scroll();
        move_cursor();
        return;
    }
    VGA_MEM[cursor_y*VGA_WIDTH + cursor_x] = (uint16_t)(c | (VGA_COLOR << 8));
    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0; cursor_y++;
        scroll();
    }
    move_cursor();
}

void prints(const char *s) { 
    while (*s) putchar(*s++); 
}

void clear_screen() {
    for (int i = 0; i < VGA_WIDTH*VGA_HEIGHT; i++)
        VGA_MEM[i] = (uint16_t)(' ' | (VGA_COLOR << 8));
    cursor_x = cursor_y = 0;
    move_cursor();
}
