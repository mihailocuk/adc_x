/* 
 * File:   ADCmain.c
 * Author: Mihailo Cuk
 * Program Title: Analog-to-Digital Conversion
 * Description: Mobile alchohol tester with ADC, UART, and servo control
 */
#include <stdio.h>               // Standard I/O
#include <stdlib.h>              // Standard library (EXIT_SUCCESS)
#include <p30fxxxx.h>            // Device-specific register definitions
#include "adc.h"                // ADC initialization and configuration (external file)
#include "timer1.h"             // Timer1 control (external file)
#include "timer2.h"             // Timer2 control (external file)

// CONFIGURATION BITS
// XT Oscillator with PLLx4 configuration:
// - XT: Uses external crystal oscillator (e.g., 10 MHz)
// - PLLx4: Multiplies the input clock by 4 -> Fosc = 10 MHz × 4 = 40 MHz
// - Fosc (system clock) = 40 MHz
// - Fcy (instruction cycle clock) = Fosc / 4 = 10 MHz
//   → Each instruction cycle takes 100 ns
//   → This frequency affects UART baud rate, timers, ADC sampling, etc.
_FOSC(CSW_FSCM_OFF & XT_PLL4);  // Oscillator configuration
_FWDT(WDT_OFF);                 // Watchdog timer OFF

// Global variables for timing
unsigned int counter_ms = 0;     // Millisecond counter - timer 1 frequency define in main function with initTIMER1(10000)
unsigned int counter_us = 0;     // Microsecond counter - timer 2 frequency define in main function with initTIMER2(100)

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
void Delay_ms(int vreme) {
    counter_ms = 0;
    while (counter_ms < vreme);
}

// Blocking delay using us counter
void Delay_us(int vreme) {
    counter_us = 0;
    while (counter_us < vreme);
}

// Initialize pins RB6, RB7 for servo control
void pinInit() {
    ADPCFGbits.PCFG6 = 1;       // Set RB6 as digital
    ADPCFGbits.PCFG7 = 1;       // Set RB7 as digital
    TRISBbits.TRISB6 = 0;       // RB6 as output
    TRISBbits.TRISB7 = 0;       // RB7 as output
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

// Buzzer function (square wave generation using toggling)
void buzzer() {
    int j, ton, duz;
    for (j = 0; j < 30; j++) {
        for (ton = 0; ton < 70; ton++) {
            LATAbits.LATA11 = ~LATAbits.LATA11;  // Toggle buzzer pin
            for (duz = 0; duz < 100; duz++);     // Short delay
        }
        Delay_ms(50);
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