#include "includes.h"

/* ************************************************************************** */

void delay_init(void)
{
    timer1_init();
    timer1_clear();
    TIMER1_IF = 0;
}

void delay_us(uint16_t useconds)
{
    useconds >>= 1;
    while (useconds > 0)
    {
        useconds--;
        asm("nop");
        asm("nop");
    }
}

void delay_ms(uint16_t mseconds)
{
    timer1_start();
    
    while (mseconds > 0)
    {
        TIMER1_IF = 0;
        TMR1H = 0xE0;
        TMR1L = 0xC0;
        
        while ((TIMER1_IF) == 0);
        mseconds--;
    }
    timer1_stop();
}
