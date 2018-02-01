#include "includes.h"

/* ************************************************************************** */

void delay_init(void)
{
    timer3_init();
    timer3_clear();
    TIMER3_IF = 0;
}

// void delay_us(uint16_t useconds)
// {
//     useconds >>= 1;
//     while (useconds > 0)
//     {
//         useconds--;
//         __asm__ ("nop");
//     }
// }

void delay_10us(uint16_t useconds)
{
    timer3_start();
    
    while (useconds > 0)
    {
        TIMER3_IF = 0;
        TMR3H = 0xFF;
        TMR3L = 0xC1;
        
        while ((TIMER3_IF) == 0);
        useconds--;
    }
    timer3_stop();
}

void delay_100us(uint16_t useconds)
{
    timer3_start();
    
    while (useconds > 0)
    {
        TIMER3_IF = 0;
        TMR3H = 0xFD;
        TMR3L = 0x05;
        
        while ((TIMER3_IF) == 0);
        useconds--;
    }
    timer3_stop();
}

void delay_ms(uint16_t mseconds)
{
    timer3_start();
    
    while (mseconds > 0)
    {
        TIMER3_IF = 0;
        TMR3H = 0xE0;
        TMR3L = 0xC0;
        
        while ((TIMER3_IF) == 0);
        mseconds--;
    }
    timer3_stop();
}
