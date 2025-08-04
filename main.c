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
#include "uart.h"
#include "driverGLCD.h"

#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14
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
unsigned int temp0, temp1, temp2, temp3; // Temporary variables for ADC results
unsigned int touch_X, touch_Y, mq3, fotores; // Raw ADC results
unsigned int pir, pijan, warning; // PIR sensor state and alcohol detection

unsigned int X, Y,x_vrednost, y_vrednost; // Touch coordinates

const unsigned int X_min=220,X_max=3642,Y_min=520,Y_max=3450; // Touch calibration values
/*const*/ unsigned int mq3_MAX, fotores_MAX; // Thresholds for MQ3 and photoresistor

void pinInit() {
// Initialize pins - GLCD IS TAKING PINS RB0-RB5 and RD0-RD3
    // SERVO
    TRISDbits.TRISD9 = 0;       // RD9 as output

    // BUZZER
    TRISAbits.TRISA11 = 0;      // A11 as output

    // Sensors
    ADPCFGbits.PCFG6 = 0; //Set RB7 as analog (FOTORESISTOR)
    TRISBbits.TRISB6 = 1; // RB7 as input (FOTORESISTOR)

    ADPCFGbits.PCFG7 = 0; //Set RB8 as analog (MQ3 sensor)
    TRISBbits.TRISB7 = 1; // RB8 as input (MQ3 sensor)
    
    TRISDbits.TRISD8 = 1; // D8 as input (PIR sensor)
    
    // TOUCH
    ADPCFGbits.PCFG8 = 0; //pin_B8_analogni
    ADPCFGbits.PCFG9 = 0; //pin_B9_analogni
    TRISCbits.TRISC13= 0;
    TRISCbits.TRISC14= 0;
    TRISBbits.TRISB8 = 1; //pin_B8_ulaz(touchX)
    TRISBbits.TRISB9 = 1; //pin_B9_ulaz(touchY)

}

// Timer1 ISR - triggered every 1ms - defined in main()
void __attribute__((__interrupt__)) _T1Interrupt(void) {
    TMR1 = 0;                   // Reset Timer1
    counter_ms++;               // Increment ms counter
    IFS0bits.T1IF = 0;          // Clear interrupt flag
}

// Timer2 ISR - triggered every 1us - defined in main()
void __attribute__((__interrupt__)) _T2Interrupt(void) {
    TMR2 = 0;                   // Reset Timer2
    counter_us++;               // Increment us counter
    IFS0bits.T2IF = 0;          // Clear interrupt flag
}

void __attribute__((__interrupt__)) _ADCInterrupt(void) {	
    						
    fotores=ADCBUF0; 
	mq3=ADCBUF1;
    touch_X=ADCBUF2;
    touch_Y=ADCBUF3;	
    										
	temp0=fotores;  //Touch X
	temp1=mq3;  //Touch Y
    temp2=touch_X;  //fotoresistor	
    temp3=touch_Y;      //MQ3 sensor
    IFS0bits.ADIF = 0;

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


// SERVO control functions (by PWM simulation using delays)
void servo_0() {
    LATDbits.LATD9 = 1;
    Delay_ms(1);
    LATDbits.LATD9 = 0;
    Delay_ms(19);
}

void servo_180() {
    LATDbits.LATD9 = 1;
    Delay_ms(2);
    LATDbits.LATD9 = 0;
    Delay_ms(18);
}

void pir_senzor() {
    if(PORTDbits.RD8==1) {
        pir=1; 
    }
    else {
        pir=0;
    }
}

void mq3_sensor() {
    if(temp1>0 && temp1<mq3_MAX) {
        pijan=0;
    }
    else {
        pijan=1;
    }
}

void fotoresistor(){
    if(temp0>0 && temp0<fotores_MAX) {
        warning=0; // Normal light level
    }
    else {
        warning=1; // Low light level
    }
}

void Touch_Panel (void)
{
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay_ms(50); 				
	
	x_vrednost = temp2;	

	
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay_ms(50); 
    
	y_vrednost = temp3;
	
    X=(x_vrednost-161)*0.03629;
	Y= ((y_vrednost-500)*0.020725);
}


// MAIN PROGRAM
int main(int argc, char** argv) {
    pinInit();                  // Initialize pins
    ConfigureLCDPins();         // Initialize LCD pins
    initTIMER1(10000);          // 1 ms period (10,000 counts at 10 MHz)
    initTIMER2(10);             // 1 us period (100 counts at 10 MHz)

    Delay_ms(1000);

    GLCD_LcdInit();             // Initialize GLCD
    Delay_ms(100);              // Wait for GLCD to stabilize
    GLCD_ClrScr();              // Clear GLCD    
    initUART1();                // Initialize
    ADCinit();                  // Initialize ADC
    ADCON1bits.ADON = 1;        // Start ADC module

    servo_180();

    while (1) {
        // 1. Provera PIR senzora
        pir_senzor();
        if (pir == 1) {
            playBuzzerApprove();        // Zvučni signal - detekcija pokreta

            servo_0();                  // Rotiraj servo ka korisniku
            Delay_ms(500);
            playBuzzerApprove();      // Zvučni signal - pozicioniran

            

            
            void funkcija(){//ispis na glcd
            mq3_sensor();
            if(pijan==1){
                playBuzzerDecline();
                //ispis na glcd
                LcdSelectSide(RIGHT);
                GLCD_Rectangle(0, 63, 0, 63);
                Delay_us(40);
                GoToXY(22, 30);
                GLCD_Printf("Try Again?"); // Ispis na GLCD
                LcdSelectSide(LEFT);
                GLCD_Rectangle(0, 63, 0, 63);
                GoToXY(22, 30);
                GLCD_Printf("Cancel"); // Ispis na GLCD
                Delay_us(40);
                Touch_Panel(); 
                if(X>0 && X<64 && Y>0 && Y<64) {
                    playBuzzerApprove();
                    GLCD_ClrScr();
                    funkcija();
                }
                if(X>64 && X<128 && Y>0 && Y<64) {
                    GLCD_ClrScr();
                    GoToXY(32, 29);
                    GLCD_Printf("Access Denied");
                    Delay_ms(1000);
                    GLCD_ClrScr();
                    // Reset servo position
                    servo_180();
                }
                //pristup odbijen, pokusaj ponovo, cancel - sve na glcdu
            }

            if(pijan==0){
                playBuzzerApprove();
                GLCD_ClrScr();
                GoToXY(32, 29);
                GLCD_Printf("Access Granted");
                playBuzzerApprove();
                Delay_ms(1000);
                servo_180();
            }
        }

        }

        Delay_ms(100); // mala pauza u petlji da ne lupa stalno
    }

    return (EXIT_SUCCESS);
}
    