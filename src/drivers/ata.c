#include "ata.h"
#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port));
}

static inline void outw(uint16_t port, uint16_t val) {
    asm volatile ("outw %%ax, %%dx" :: "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

static void ata_wait() {
    while ((inb(0x1F7) & 0xC0) != 0x40);
}

int ata_write_sector(uint32_t lba, const uint8_t* buf) {
    ata_wait();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x30);
    ata_wait();
    for (int i = 0; i < 256; i++) {
        uint16_t w = buf[2*i] | (buf[2*i+1] << 8);
        outw(0x1F0, w);
    }
    ata_wait();
    return 1;
}

int ata_read_sector(uint32_t lba, uint8_t* buf) {
    ata_wait();
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));
    outb(0x1F7, 0x20);
    ata_wait();
    for (int i = 0; i < 256; i++) {
        uint16_t w;
        asm volatile ("inw %%dx, %%ax" : "=a"(w) : "d"(0x1F0));
        buf[2*i] = w & 0xFF;
        buf[2*i+1] = (w >> 8) & 0xFF;
    }
    return 1;
}

int detect_hdd() {
    outb(0x1F6, 0xA0);
    for (volatile int i = 0; i < 1000; i++);
    uint8_t status = inb(0x1F7);
    if (status == 0xFF) return 0;
    for (int i = 0; i < 4; i++) {
        status = inb(0x1F7);
        if ((status & 0xC0) == 0x40) break;
        for (volatile int j = 0; j < 10000; j++);
    }
    return (status & 0xC0) == 0x40;
}
