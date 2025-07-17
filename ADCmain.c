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


_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal 10MHz
_FWDT(WDT_OFF);


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
    
    for(broj1=0;broj1<10000;broj1++);

		TRISBbits.TRISB0=1;
		TRISBbits.TRISB1=1;
        TRISBbits.TRISB2=1;

		for(broj=0;broj<60000;broj++);


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

