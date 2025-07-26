/* 
 * File:   ADCmain.c
 * Author: Mihailo Cuk
 * Program Title: Analog-to-Digital Conversion
 * Description: Mobile alchohol tester with ADC, UART, and servo control
 */
#include <stdio.h>               // Standard I/O
#include <stdlib.h>              // Standard library (EXIT_SUCCESS)
#include <p30fxxxx.h>            // Device-specific register definitions
#include "adc.h"                // ADC initialization and configuration
#include "timer1.h"             // Timer1 control
#include "timer2.h"             // Timer2 control
#include "buzzer.h"             // Buzzer control

/***************************************************************************
 * Clock Explanation for dsPIC33 (XT + PLL4 Configuration)
 *
 * Fxt  = 10 MHz      → External crystal oscillator
 * PLL  = ×4          → Multiplies Fxt → Fosc = 10 MHz × 4 = 40 MHz
 * Fcy  = Fosc / 4    → Instruction cycle clock = 40 MHz / 4 = 10 MHz
 *
 * Why Fcy = Fosc / 4?
 * --------------------
 * - The dsPIC CPU uses a 4-stage instruction pipeline:
 *     1. Fetch
 *     2. Decode
 *     3. Read
 *     4. Execute
 *   → One instruction completes every 4 system clock cycles (Fosc),
 *     which defines the instruction clock: Fcy = Fosc / 4
 *
 * - Fcy is used by:
 *     • Timers (prescalers, overflow rates)
 *     • UART (baud rate calculations)
 *     • ADC sampling and conversion timing
 *     • Other peripherals relying on instruction-rate timing
 *
 * - Fixed division by 4 ensures:
 *     • Predictable timing formulas
 *     • Compatibility with older PIC devices
 *
 * Example:
 *   - Baud rate = Fcy / (16 × UxBRG)
 *   - Timer delay = (PRx + 1) / Fcy
 ***************************************************************************/
_FOSC(CSW_FSCM_OFF & XT_PLL4);  // Configure oscillator: XT mode with 4× PLL
_FWDT(WDT_OFF);                 // Disable Watchdog Timer

// Global variables for timing (updated by Timer1 and Timer2 ISRs)
unsigned int counter_ms = 0;  // Millisecond counter (Timer1 must be initialized with initTIMER1(10000))
unsigned int counter_us = 0;  // Microsecond counter (Timer2 must be initialized with initTIMER2(100))


// Timer1 ISR - triggered every 1ms
void __attribute__((__interrupt__)) _T1Interrupt(void) {
    TMR1 = 0;                   // Reset Timer1
    counter_ms++;               // Increment ms counter
    IFS0bits.T1IF = 0;          // Clear interrupt flag
}

// Timer2 ISR - triggered every 10us
void __attribute__((__interrupt__)) _T2Interrupt(void) {
    TMR2 = 0;                   // Reset Timer2
    counter_us++;               // Increment us counter
    IFS0bits.T2IF = 0;          // Clear interrupt flag
}

// Blocking delay using ms counter
void Delay_ms(int time) {
    counter_ms = 0;
    while (counter_ms < time);
}

// Blocking delay using us counter
void Delay_us(int time) {
    counter_us = 0;
    while (counter_us < time);
}

// Initialize pins RB6, RB7 for servo control
void pinInit() {
    // SERVO pins initialization
    ADPCFGbits.PCFG6 = 1;       // Set RB6 as digital
    ADPCFGbits.PCFG7 = 1;       // Set RB7 as digital
    TRISBbits.TRISB6 = 0;       // RB6 as output
    TRISBbits.TRISB7 = 0;       // RB7 as output

    // BUZZER pin initialization
    TRISAbits.TRISA11 = 0;      // A11 as output
}

// SERVO control functions (by PWM simulation using delays)
void servo_device_0() {
    LATBbits.LATB6 = 1;
    Delay_ms(1);
    LATBbits.LATB6 = 0;
    Delay_ms(19);
}

void servo_device_180() {
    LATBbits.LATB6 = 1;
    Delay_ms(2);
    LATBbits.LATB6 = 0;
    Delay_ms(18);
}

void servo_door_0() {
    LATBbits.LATB7 = 1;
    Delay_ms(1);
    LATBbits.LATB7 = 0;
    Delay_ms(19);
}

void servo_door_180() {
    LATBbits.LATB7 = 1;
    Delay_ms(2);
    LATBbits.LATB7 = 0;
    Delay_ms(18);
}

/***************************************************************************
* Function Name    : playBuzzerAlert
* Description      : Generates a beeping sound by toggling pin RA11 at
*                    a fixed frequency. Produces 30 short bursts.
* Parameters       : None
* Return Value     : None
* Notes            :
*   - The tone is created by toggling the pin at a fixed rate (square wave).
*   - A short delay between toggles simulates frequency.
*   - RA11 pin must be configured as output beforehand.
***************************************************************************/
void playBuzzerAlert(void) {
    const int numberOfBeeps = 30;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 100;  // Roughly controls pitch
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;                   // Toggle RA11 pin
            for (int delayCycle = 0; delayCycle < toggleDelayCycles; delayCycle++);  // Short delay loop
        }
        Delay_ms(interBeepDelay_ms);                // Delay between beeps
    }
}

/***************************************************************************
* Function Name    : playBuzzerApprove
* Description      : Generates two short beeps at normal pitch to indicate approval.
***************************************************************************/
void playBuzzerApprove(void) {
    const int numberOfBeeps = 2;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 100;  // Normal pitch
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;                    // Toggle RA11 pin
            for (int delayCycle = 0; delayCycle < toggleDelayCycles; delayCycle++);  // Pitch control
        }
        Delay_ms(interBeepDelay_ms);
    }
}

/***************************************************************************
* Function Name    : playBuzzerDecline
* Description      : Generates three short beeps at higher pitch to indicate decline.
***************************************************************************/
void playBuzzerDecline(void) {
    const int numberOfBeeps = 3;
    const int togglesPerBeep = 70;
    const int toggleDelayCycles = 50;   // Higher pitch = faster toggling
    const int interBeepDelay_ms = 50;

    for (int beep = 0; beep < numberOfBeeps; beep++) {
        for (int toggle = 0; toggle < togglesPerBeep; toggle++) {
            LATAbits.LATA11 ^= 1;
            for (int delayCycle = 0; delayCycle < toggleDelayCycles; delayCycle++);
        }
        Delay_ms(interBeepDelay_ms);
    }
}

// ADC result variables
unsigned int sirovi0, sirovi1, sirovi2;
unsigned int broj, broj1, broj2, tempRX;

/***************************************************************************
* Function Name    : initUART1
* Description      : Initializes UART1 module for RS232 communication 
*                    with 9600 baud rate and alternate I/O pins.
* Parameters       : None
* Return Value     : None
***************************************************************************/
void initUART1(void)
{
    U1BRG = 0x0040;             // Baud rate generator for 9600 baud @ Fcy = 10 MHz
    U1MODEbits.ALTIO = 1;       // Use alternate I/O pins for TX/RX (U1ATX, U1ARX)
    IEC0bits.U1RXIE = 1;        // Enable UART1 receive interrupt
    U1STA &= 0xFFFC;            // Clear status register bits (URXISEL)
    U1MODEbits.UARTEN = 1;      // Enable UART1 module
    U1STAbits.UTXEN = 1;        // Enable UART1 transmitter
}

// UART RX interrupt (currently unused)
void __attribute__((__interrupt__)) _U1RXInterrupt(void) {
    IFS0bits.U1RXIF = 0;        // Clear interrupt flag
    // tempRX = U1RXREG;        // Read received data if needed
}

// ADC interrupt handler
void __attribute__((__interrupt__)) _ADCInterrupt(void) {
    sirovi0 = ADCBUF0;          // Read analog value from AN0 (RB0)
    sirovi1 = ADCBUF1;          // Read from AN1 (RB1)
    sirovi2 = ADCBUF2;          // Read from AN2 (RB2)
    IFS0bits.ADIF = 0;          // Clear ADC interrupt flag
}

/*********************************************************************
* Function Name    : WriteUART1
* Description      : Sends a single byte via UART1 by writing it 
*                    into the U1TXREG register.
* Parameters       : data - The byte to be sent (only LSB is used 
*                    if UART is in 8-bit mode).
* Return Value     : None
*********************************************************************/
void WriteUART1(unsigned int data)
{
    while (U1STAbits.TRMT == 0); // Wait until transmit shift register is empty

    if (U1MODEbits.PDSEL == 3)   // If UART is in 9-bit mode
        U1TXREG = data;
    else                         // Otherwise, send lower 8 bits
        U1TXREG = data & 0xFF;
}


/***********************************************************************
* Function Name    : WriteUART1dec2string
* Description      : Sends a 4-digit number over UART1 by converting 
*                    each digit into ASCII and sending it one-by-one.
* Parameters       : data - The number to be sent (0–9999)
* Return Value     : None
************************************************************************/
void WriteUART1dec2string(unsigned int data)
{
    unsigned char digit;

    digit = data / 1000;
    WriteUART1(digit + '0');      // Thousands digit
    data = data % 1000;

    digit = data / 100;
    WriteUART1(digit + '0');      // Hundreds digit
    data = data % 100;

    digit = data / 10;
    WriteUART1(digit + '0');      // Tens digit

    WriteUART1((data % 10) + '0'); // Units digit
}


// MAIN PROGRAM
int main(int argc, char** argv) {
    pinInit();                     // Init servo pins
    initTIMER1(10000);            // 1 ms period (10,000 counts at 10 MHz)
    initTIMER2(10);              // 1 us period (100 counts at 10 MHz)

    for (broj1 = 0; broj1 < 10000; broj1++); // Startup delay

    TRISBbits.TRISB0 = 1;         // AN0 as input
    TRISBbits.TRISB1 = 1;         // AN1 as input
    TRISBbits.TRISB2 = 1;         // AN2 as input

    for (broj = 0; broj < 60000; broj++);   // Additional delay

    initUART1();                  // Start UART
    ADCinit();                    // Initialize ADC
    ADCON1bits.ADON = 1;         // Start ADC module

    while (1) {
        WriteUART1(' ');
        WriteUART1dec2string(sirovi0);      // Send AN0 value
        for (broj2 = 0; broj2 < 1000; broj2++);

        WriteUART1(' ');
        for (broj2 = 0; broj2 < 1000; broj2++);
        WriteUART1dec2string(sirovi1);      // Send AN1 value
        for (broj2 = 0; broj2 < 1000; broj2++);

        WriteUART1(' ');
        WriteUART1dec2string(sirovi2);      // Send AN2 value
        for (broj2 = 0; broj2 < 1000; broj2++);

        WriteUART1(13);                     // Carriage return
        WriteUART1('s');                    // Send 's' as end-of-frame marker

        for (broj1 = 0; broj1 < 1000; broj1++)
        for (broj2 = 0; broj2 < 3000; broj2++); // Delay
    }

    return (EXIT_SUCCESS);
}