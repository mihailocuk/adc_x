#include "timer1.h"
#include<p30fxxxx.h>
#ifdef _T1IF

/********************************************************************
*    Function Name:  CloseTimer1                                    *
*    Description:    This routine disables the Timer1 and its       *
*                    interrupt enable and flag bits.                *
*    Parameters:     None                                           *
*    Return Value:   None                                           *
********************************************************************/

void CloseTimer1(void)
{
    _T1IE = 0;      /* Disable the Timer1 interrupt */
    T1CONbits.TON = 0;      /* Disable timer1 */
    _T1IF = 0;      /* Clear Timer interrupt flag */
}


/*******************************************************************
*    Function Name: ConfigIntTimer1                                *
*    Description:   This function configures interrupt and sets    *
*                   Interrupt Priority                             *
*    Return Value:  None                                           *
*******************************************************************/

void ConfigIntTimer1(unsigned int config)
{
    _T1IF = 0;                   /* clear IF bit */
    _T1IP = (config &0x0007);    /* assigning Interrupt Priority */
    _T1IE = (config &0x0008)>>3; /* Interrupt Enable /Disable */
}



/********************************************************************
*    Function Name:  OpenTimer1                                     *
*    Description:    This routine configures the timer control      *
*                    register and timer period register.            *
*    Parameters:     config: bit definitions to configure Timer1    *
*                    period: Value to be loaded to PR reg           *
*    Return Value:   None                                           *
********************************************************************/
void OpenTimer1(unsigned int config,unsigned int period)
{
    TMR1  = 0;          /* Reset Timer1 to 0x0000 */
    PR1   = period;     /* assigning Period to Timer period register */
    T1CON = config;     /* Configure timer control reg */
}

/********************************************************************
*    Function Name:  ReadTimer1                                     *
*    Description:    This routine reads the 16-bit value from       *
*                    Timer Register.                                *
*    Parameters:     None                                           *
*    Return Value:   unsigned int: Timer  16-bit value              *
********************************************************************/

unsigned int ReadTimer1(void)
{
    return (TMR1);      /* Return the Timer1 register */
}


/********************************************************************
*    Function Name:  WriteTimer1                                    *
*    Description:    This routine writes a 16-bit value to Timer1   *
*    Parameters:     unsigned int: value to write to Timer          *
*    Return Value:   None                                           *
********************************************************************/

void WriteTimer1(unsigned int timer)
{
    TMR1 = timer;     /* assign timer value to Timer1 Register */
}

void initTIMER1(int period)//konfiguracija Timer1
{
	unsigned int match_value;//vrednost koja se stavlja u period

	ConfigIntTimer1(T1_INT_PRIOR_1 & T1_INT_ON);//prioritet i ukljucivanje tajmera
	WriteTimer1(0);
	match_value = period ;
	OpenTimer1(T1_ON & T1_GATE_OFF & T1_IDLE_CON & T1_PS_1_1 & T1_SYNC_EXT_OFF & T1_SOURCE_INT,match_value );

}

#else 
#warning "Does not build on this target"
#endif
