#include "includes.h"

/* ************************************************************************** */

void delay_us(uint16_t useconds)
{
    useconds >>= 1;
    while (useconds > 0)
    {
        useconds--;
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

void delay_ms(uint16_t mseconds)
{
    uint24_t startTime = systick_read();

    while(1)
    {
        uint24_t currentTime = systick_read();
        uint24_t elapsedTime = currentTime - startTime;
        if((elapsedTime) >= mseconds) break;
    }
}