/* 
 * File:   uart.h
 * Author: Vincilir
 *
 * Created on January 6, 2022, 4:43 PM
 */

#ifndef UART_H
#define	UART_H

#ifdef	__cplusplus
extern "C" {
#endif


#include<p30fxxxx.h>
void initUART1(void);
void WriteUART1(unsigned int data);
void RS232_putst(register const char *str);
void WriteUART1dec2string(unsigned int data);


#ifdef	__cplusplus
}
#endif

#endif	/* UART_H */

