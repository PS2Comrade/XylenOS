#include "reboot.h"
#include <stdint.h>

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port));
}

void do_reboot() {
    while (inb(0x64) & 0x02) {}
    outb(0x64, 0xFE);
    for (;;) asm volatile ("hlt");
}
