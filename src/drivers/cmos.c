#include "cmos.h"
#include "vga.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

static uint8_t bcd_to_bin(uint8_t bcd) { 
    return (bcd & 0x0F) + ((bcd >> 4) * 10); 
}

static uint8_t read_cmos(uint8_t reg) {
    outb(0x70, reg); 
    return inb(0x71);
}

static void print_num2(int num) {
    putchar('0' + (num / 10)); 
    putchar('0' + (num % 10));
}

void print_time() {
    uint8_t sec = read_cmos(0x00), min = read_cmos(0x02), hour = read_cmos(0x04);
    uint8_t day = read_cmos(0x07), mon = read_cmos(0x08), year = read_cmos(0x09);
    uint8_t status_b = read_cmos(0x0B);
    if (!(status_b & 0x04)) {
        sec = bcd_to_bin(sec); min = bcd_to_bin(min); hour = bcd_to_bin(hour);
        day = bcd_to_bin(day); mon = bcd_to_bin(mon); year = bcd_to_bin(year);
    }
    prints("20"); print_num2(year); putchar('-');
    print_num2(mon); putchar('-'); print_num2(day); putchar(' ');
    print_num2(hour); putchar(':'); print_num2(min); putchar(':'); print_num2(sec); putchar('\n');
}
