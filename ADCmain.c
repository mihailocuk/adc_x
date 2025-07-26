/* 
 * File:   ADCmain.c
 * Author: dr Vladimir Rajs
 * Naziv programa  :  Analogno digitalna konverzija
 * Primena programa:  Program za inicijalizaciju AD konverzije mikrokontrolera dsPIC30f4013
 *                    koja se nadgleda preko serijske RS232 komunikacije
 *                    (baud rate 9600 i oscilator 10MHz sa ukljucenim PLL4)
 *Opis programa   :  Vrednosti sa prva tri AD konvertora (RB0,RB1,RB2) i karakter 's' salje na PC 
                      preko serijske RS232 komunikacije  
 * 
 * Created on 27. septembar 2017., 16.07
 */

#include <stdio.h>
#include <stdlib.h>
#include <p30fxxxx.h>
#include "adc.h"
#include "timer1.h"
#include "timer2.h"


_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal 10MHz
_FWDT(WDT_OFF);

unsigned int brojac_ms = 0;
unsigned int brojac_us = 0;

void __attribute__((__interrupt__)) _T1Interrupt(void) // u main-u definishanje periode tako da bude ms
{
   	TMR1 =0;
	brojac_ms++;//brojac milisekundi
    IFS0bits.T1IF = 0;   
}

void __attribute__((__interrupt__)) _T2Interrupt(void) // u main-u definishanje periode tako da bude us
{
   	TMR2 =0;
	brojac_us++;//brojac mikrosekundi
    IFS0bits.T2IF = 0;   
}

void Delay_ms (int vreme)
{
    brojac_ms = 0;
    while(brojac_ms < vreme);
}

void Delay_us (int vreme) // ZA GLCD
{
    brojac_us = 0;
    while(brojac_us < vreme);
}

void pinInit() {
    //SERVO//
    ADPCFGbits.PCFG6 = 1;
    ADPCFGbits.PCFG7 = 1;
    TRISBbits.TRISB6 = 0;
    TRISBbits.TRISB7 = 0;
}

// FUNKCIJE AKTUATORA //

    void servo_device_0()
    {
        LATBbits.LATB6=1;
        Delay_ms (1); 
        LATBbits.LATB6=0;
        Delay_ms (19);
    }
    void servo_device_180()
    {
        LATBbits.LATB6=1;
        Delay_ms (2);
        LATBbits.LATB6=0;
        Delay_ms (18);
    }

    void servo_door_0()
{
    LATBbits.LATB7=1;
    Delay_ms (1); 
    LATBbits.LATB7=0;
    Delay_ms (19);
}
    void servo_door_180()
{
    LATBbits.LATB7=1;
    Delay_ms (2); 
    LATBbits.LATB7=0;
    Delay_ms (18);  
}

void buzzer()
{
    for(j = 0; j < 30; j++)
    {
        for(ton = 0; ton < 70; ton++)
        {
            LATAbits.LATA11 =~ LATAbits.LATA11;
            for(duz = 0;duz < 100; duz++);                    
        }
        Delay_ms(50);
    }
}


unsigned int sirovi0,sirovi1,sirovi2,sirovi3;
unsigned int broj,broj1,broj2,tempRX;
/***************************************************************************
* Ime funkcije      : initUART1                                            *
* Opis              : inicjalizuje RS232 komunikaciju s 9600bauda          * 
* Parameteri        : Nema                                                 *
* Povratna vrednost : Nema                                                 *
***************************************************************************/

void initUART1(void)
{
U1BRG=0x0040;//baud rate 9600
U1MODEbits.ALTIO = 1;
IEC0bits.U1RXIE = 1;
U1STA&=0xfffc;
U1MODEbits.UARTEN=1;
U1STAbits.UTXEN=1;
}
void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
  //   tempRX=U1RXREG;

} 

void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
							

										sirovi0=ADCBUF0;
										sirovi1=ADCBUF1;
										sirovi2=ADCBUF2;
										

    IFS0bits.ADIF = 0;

} 

/*********************************************************************
* Ime funkcije      : WriteUART1                            		 *
* Opis              : Funkcija upisuje podatke u registar U1TXREG,   *
*                     za slanje podataka    						 *
* Parameteri        : unsigned int data-podatak koji zelimo poslati  *
* Povratna vrednost : Nema                                           *
*********************************************************************/

void WriteUART1(unsigned int data)
{


	while (U1STAbits.TRMT==0);
    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}
/***********************************************************************
* Ime funkcije      : WriteUART1dec2string                     		   *
* Opis              : Funkcija salje 4-cifrene brojeve (cifru po cifru)*
* Parameteri        : unsigned int data-podatak koji zelimo poslati    *
* Povratna vrednost : Nema                                             *
************************************************************************/
void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}


/*
 * 
 */
int main(int argc, char** argv) {
    
    pinInit();
    initTIMER1(10000); // 10000 = 1 ms
    initTIMER2(100); // 100 = 10 us
    
    for(broj1=0; broj1<10000; broj1++);

		TRISBbits.TRISB0=1;
		TRISBbits.TRISB1=1;
        TRISBbits.TRISB2=1;

		for(broj=0; broj<60000; broj++);


		initUART1();//inicijalizacija UART-a
 		ADCinit();//inicijalizacija AD konvertora

		ADCON1bits.ADON=1;//pocetak Ad konverzije 
	while(1)
	{

		WriteUART1(' ');
		WriteUART1dec2string(sirovi0);
		for(broj2=0;broj2<1000;broj2++);
		WriteUART1(' ');
		for(broj2=0;broj2<1000;broj2++);
		WriteUART1dec2string(sirovi1);
		for(broj2=0;broj2<1000;broj2++);
		WriteUART1(' ');
		WriteUART1dec2string(sirovi2);
		for(broj2=0;broj2<1000;broj2++);
		WriteUART1(13);//enter

		WriteUART1('s');


		for(broj1=0;broj1<1000;broj1++)
		for(broj2=0;broj2<3000;broj2++);

	}//od whilea

    return (EXIT_SUCCESS);
}

