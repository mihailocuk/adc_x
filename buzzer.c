// File: buzzer.c
// Description: Buzzer sound functions for generating different tones
//              using a digital output pin (RA11) to simulate square wave beeping.
// Author: Mihailo Cuk

#include <p30fxxxx.h>     // Include device-specific header for dsPIC30F series
#include "buzzer.h"       // Include corresponding header for function declarations

// External function to generate millisecond delays
// This function is defined in the main program (e.g., ADCmain.c)
extern void Delay_ms(int time);

/***************************************************************************
 * Function Name    : shortDelay
 * Description      : Provides a short busy-wait delay used for tone frequency control.
 * Parameters       : cycles - number of loop iterations (controls delay length)
 * Return Value     : None
 * Notes            :
 *   - The function is static since it's only used internally in this file.
 *   - Used to adjust the pitch of the beeping sound.
 ***************************************************************************/
static void shortDelay(int cycles) {
    int i;
    for (i = 0; i < cycles; i++);  // Simple for-loop delay
}

/***************************************************************************
 * Function Name    : playBuzzerAlert
 * Description      : Produces 30 short beeps at normal pitch to alert the user.
 * Parameters       : None
 * Return Value     : None
 * Notes            :
 *   - Toggles RA11 to generate a square wave (sound signal).
 *   - Uses busy-wait loops to simulate frequency and timing.
 ***************************************************************************/
void playBuzzerAlert(void) {
    const int numberOfBeeps = 30;        // Total number of beeps to produce
    const int togglesPerBeep = 70;       // Number of toggles for each beep
    const int toggleDelayCycles = 100;   // Delay between toggles (controls pitch)
    const int interBeepDelay_ms = 50;    // Delay between each beep (ms)

    int beep, toggle;
    for (beep = 0; beep < numberOfBeeps; beep++) {
        for (toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;            // Toggle RA11 pin (on/off)
            shortDelay(toggleDelayCycles);   // Wait between toggles
        }
        Delay_ms(interBeepDelay_ms);         // Wait between beeps
    }
}

/***************************************************************************
 * Function Name    : playBuzzerApprove
 * Description      : Produces 2 short beeps at normal pitch to indicate approval.
 * Parameters       : None
 * Return Value     : None
 * Notes            :
 *   - This tone is softer and shorter, used for confirmation or approval feedback.
 ***************************************************************************/
void playBuzzerApprove(void) {
    const int numberOfBeeps = 2;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 100;   // Normal pitch
    const int interBeepDelay_ms = 50;

    int beep, toggle;
    for (beep = 0; beep < numberOfBeeps; beep++) {
        for (toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;        // Toggle RA11 pin
            shortDelay(toggleDelayCycles);
        }
        Delay_ms(interBeepDelay_ms);
    }
}

/***************************************************************************
 * Function Name    : playBuzzerDecline
 * Description      : Produces 3 short beeps at higher pitch to indicate a decline or error.
 * Parameters       : None
 * Return Value     : None
 * Notes            :
 *   - Higher pitch achieved by decreasing delay between toggles.
 *   - Used to signal rejection, failure, or error.
 ***************************************************************************/
void playBuzzerDecline(void) {
    const int numberOfBeeps = 3;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 50;    // Faster toggling → higher pitch
    const int interBeepDelay_ms = 50;

    int beep, toggle;
    for (beep = 0; beep < numberOfBeeps; beep++) {
        for (toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;        // Toggle RA11 pin
            shortDelay(toggleDelayCycles);
        }
        Delay_ms(interBeepDelay_ms);
    }
}
