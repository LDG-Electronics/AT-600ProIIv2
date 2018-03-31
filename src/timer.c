#include "includes.h"

/* ************************************************************************** */

/*  Timer0 is used in the stopwatch module

*/

void timer0_clear(void)
{
    TMR0H = 0x00;
    TMR0L = 0x00;
}

uint16_t timer0_read(void)
{
    uint16_t retval;
    
    retval = (TMR0H << 8) | TMR0L;
    return retval;
}

/*  Timer1 is unused

*/

void timer1_clear(void)
{
    TMR1H = 0x00;
    TMR1L = 0x00;
    TIMER1_IF = 0;
}

uint16_t timer1_read(void)
{
    uint16_t retval;
    
    retval = (TMR1H << 8) | TMR1L;
    return retval;
}

/*  Timer2 is used in the system tick counter

    Settings:
    Prescaler: 1:4
    Postscaler: 1:8
    TMR4 Preload: 0xF9
    Actual Interrupt Time:
    No interrupt
    Do not leave running
*/

void timer2_clear(void)
{
    TMR2 = 0x00;
}

uint16_t timer2_read(void)
{
    uint16_t retval;
    
    retval = (TMR1H << 8) | TMR1L;
    return retval;
}

/*  Timer3 is used in the frequency counter

*/

void timer3_clear(void)
{
    TMR3H = 0x00;
    TMR3L = 0x00;
    TIMER3_IF = 0;
}

uint16_t timer3_read(void)
{
    uint16_t retval;
    
    retval = (TMR3H << 8) | TMR3L;
    return retval;
}

/*  Timer4 is unused

*/

void timer4_clear(void)
{
    TMR4 = 0x00;
}

uint16_t timer4_read(void)
{
    uint16_t retval;
    
    retval = (TMR1H << 8) | TMR1L;
    return retval;
}

/*  Timer5 is used to trigger an interrupt every 5ms to poll the buttons.
    
    Settings:
    Clock Source: FOSC/4
    Prescaler: 1:2
    Preload: 25536
    Secondary Oscillator: disabled
    16-Bit Read/Write Mode: enabled
    Interrupt enabled, high priority
    Should be left running
*/

void timer5_clear(void)
{
    TMR5H = 0x63;
    TMR5L = 0xC0;
}

uint16_t timer5_read(void)
{
    uint16_t retval;
    
    retval = (TMR5H << 8) | TMR5L;
    return retval;
}

/*  Timer6 is unused

*/

void timer6_clear(void)
{
    TMR6 = 0x00;
}

uint16_t timer6_read(void)
{
    uint16_t retval;
    
    retval = (TMR1H << 8) | TMR1L;
    return retval;
}
