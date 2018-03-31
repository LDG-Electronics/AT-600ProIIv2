#include "includes.h"

/* ************************************************************************** */

/*  Timer0 is used in the stopwatch module

*/

void timer0_init(void)
{

}

void timer0_start(void)
{
    TIMER0_ON = 1;
}

void timer0_stop(void)
{
    TIMER0_ON = 0;
}

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

void timer1_init(void)
{
    
}

void timer1_start(void)
{
    TIMER1_ON = 1;
}

void timer1_stop(void)
{
    TIMER1_ON = 0;
}

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

void timer2_init(void)
{
    T2CLK = 0b00000010;
    T2CONbits.CKPS = 0b111;
    T2CONbits.OUTPS = 0b0001;

    PR2 = 0xF9;
}

void timer2_start(void)
{
    TIMER2_ON = 1;
}

void timer2_stop(void)
{
    TIMER2_ON = 0;
}

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

void timer3_init(void)
{
    T3CON = 0b00000100;
    T3CLK = 1;
}

void timer3_start(void)
{
    TIMER3_ON = 1;
}

void timer3_stop(void)
{
    TIMER3_ON = 0;
}

void timer3_clear(void)
{
    TMR3H = 0x00;
    TMR3L = 0x00;
    TIMER3_IF = 0;
}

void timer3_reset(void)
{
    timer3_stop();
    timer3_clear();
}

uint16_t timer3_read(void)
{
    uint16_t retval;
    
    retval = (TMR3H << 8) | TMR3L;
    return retval;
}

/*  Timer4 is used for the millisecond delay function

    Settings:
    Prescaler 1:4
    Postscaler 1:8
    TMR4 Preload = 250
    Actual Interrupt Time : 1.001 ms
    No interrupt
    Do not leave running
*/

void timer4_init(void)
{
    T4CON = 0b00111001;
    PR4 = 250;
}

void timer4_start(void)
{
    TIMER4_ON = 1;
}

void timer4_stop(void)
{
    TIMER4_ON = 0;
}

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

void timer5_init(void)
{
    T5CLK = 1; // Select Fosc/4 as clock source

    T5CONbits.CKPS = 0b01; // 1:2 prescale
    T5CONbits.RD16 = 1; // 16 bit mode, for atomic operation
    
    TIMER5_IF = 0;
    PIE8bits.TMR5IE = 1;
    IPR8bits.TMR5IP = 1;
}

void timer5_start(void)
{
    TIMER5_ON = 1;
}

void timer5_stop(void)
{
    TIMER5_ON = 0;
}

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

void timer6_init(void)
{
    
}

void timer6_start(void)
{
    TIMER6_ON = 1;
}

void timer6_stop(void)
{
    TIMER6_ON = 0;
}

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
