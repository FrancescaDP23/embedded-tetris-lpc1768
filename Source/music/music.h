#ifndef MUSIC_H
#define MUSIC_H

#include "LPC17xx.h"

// Definizioni di base
#define SECOND 25000000

typedef char BOOL;
#define TRUE 1
#define FALSE 0

#define BPM_SCALAR 1.0

typedef enum note_durations
{
    time_semibiscroma = (unsigned int)(SECOND / 16.0 * BPM_SCALAR), // 1/64
    time_biscroma     = (unsigned int)(SECOND / 8.0 * BPM_SCALAR),  // 1/32
    time_semicroma    = (unsigned int)(SECOND / 8.0 * BPM_SCALAR),  // 1/16 (veloce)
    time_croma        = (unsigned int)(SECOND / 4.0 * BPM_SCALAR),  // 1/8 (normale)
    time_semiminima   = (unsigned int)(SECOND / 2.0 * BPM_SCALAR),  // 1/4 (lunga)
    time_minima       = (unsigned int)(SECOND * 1.0 * BPM_SCALAR),  // 1/2
    time_semibreve    = (unsigned int)(SECOND * 2.0 * BPM_SCALAR),  // 1
} NOTE_DURATION;

typedef enum frequencies
{
    pause = 0,
    g3 = 196,
    a3b = 208,
    a3 = 220,
    b3 = 247,
    c4 = 262,
    d4 = 294,
    e4 = 330,
    f4 = 349,
    g4 = 392,
    g4s = 415,
    a4 = 440,
    b4 = 494,
    c5 = 523,
    d5 = 587,
    e5 = 659,
    f5 = 698,
    g5 = 784,
    a5 = 880
} FREQUENCY;

typedef struct
{
    FREQUENCY freq;
    NOTE_DURATION duration;
} NOTE;

extern volatile int music_cmd;

void music_init(void);
void playNote(NOTE note);
BOOL isNotePlaying(void);
void play_tick_sound(void);

#endif
