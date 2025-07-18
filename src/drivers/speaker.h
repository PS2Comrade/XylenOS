#ifndef SPEAKER_H
#define SPEAKER_H

#include "../kernel/types.h"

void speaker_init(void);
void speaker_play_freq(uint32_t frequency);
void speaker_stop(void);
void speaker_beep(uint32_t frequency, uint32_t duration_ms);
void speaker_play_melody(const uint32_t *frequencies, const uint32_t *durations, int count);

// Some predefined frequencies
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523

#endif
