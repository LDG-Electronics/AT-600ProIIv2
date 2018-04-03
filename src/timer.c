#include "includes.h"

/* ************************************************************************** */

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

/* -------------------------------------------------------------------------- */

void timer1_clear(void)
{
    TMR1H = 0x00;
    TMR1L = 0x00;
}

uint16_t timer1_read(void)
{
    uint16_t retval;
    
    retval = (TMR1H << 8) | TMR1L;
    return retval;
}

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

void timer3_clear(void)
{
    TMR3H = 0x00;
    TMR3L = 0x00;
}

uint16_t timer3_read(void)
{
    uint16_t retval;
    
    retval = (TMR3H << 8) | TMR3L;
    return retval;
}

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

void timer5_clear(void)
{
    TMR5H = 0x00;
    TMR5L = 0x00;
}

uint16_t timer5_read(void)
{
    uint16_t retval;
    
    retval = (TMR5H << 8) | TMR5L;
    return retval;
}

/* -------------------------------------------------------------------------- */

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
