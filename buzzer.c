// File: buzzer.c
#include <p30fxxxx.h>
#include "buzzer.h"

// External delay functions declared elsewhere
extern void Delay_ms(int vreme);

// Simple delay loop (for pitch control)
static void shortDelay(int cycles) {
    for (int i = 0; i < cycles; i++);
}

void playBuzzerAlert(void) {
    const int numberOfBeeps = 30;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 100;  // Normal pitch
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;
            shortDelay(toggleDelayCycles);
        }
        Delay_ms(interBeepDelay_ms);
    }
}

void playBuzzerApprove(void) {
    const int numberOfBeeps = 2;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 100;  // Normal pitch
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;
            shortDelay(toggleDelayCycles);
        }
        Delay_ms(interBeepDelay_ms);
    }
}

void playBuzzerDecline(void) {
    const int numberOfBeeps = 3;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 50;  // Higher pitch
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;
            shortDelay(toggleDelayCycles);
        }
        Delay_ms(interBeepDelay_ms);
    }
}
