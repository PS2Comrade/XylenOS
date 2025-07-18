#include "speaker.h"

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %%al, %%dx" :: "a"(val), "d"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %%dx, %%al" : "=a"(ret) : "d"(port));
    return ret;
}

static void pit_delay(uint32_t ms) {
    // Rough delay - not accurate but good enough for beeps
    for(volatile uint32_t i = 0; i < ms * 1000; i++);
}

void speaker_init(void) {
    // Nothing special needed
}

void speaker_play_freq(uint32_t frequency) {
    if(frequency == 0) {
        speaker_stop();
        return;
    }
    
    uint32_t divisor = 1193180 / frequency;
    
    // Set the PIT to the desired frequency
    outb(0x43, 0xb6);
    outb(0x42, (uint8_t)(divisor & 0xFF));
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF));
    
    // Turn on speaker
    uint8_t tmp = inb(0x61);
    if((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}

void speaker_stop(void) {
    // Turn off speaker
    uint8_t tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}

void speaker_beep(uint32_t frequency, uint32_t duration_ms) {
    speaker_play_freq(frequency);
    pit_delay(duration_ms);
    speaker_stop();
}

void speaker_play_melody(const uint32_t *frequencies, const uint32_t *durations, int count) {
    for(int i = 0; i < count; i++) {
        if(frequencies[i] == 0) {
            speaker_stop();
            pit_delay(durations[i]);
        } else {
            speaker_beep(frequencies[i], durations[i]);
        }
        pit_delay(50);  // Small gap between notes
    }
}
