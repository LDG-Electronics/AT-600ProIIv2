#include "includes.h"
#include "delay.h"

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
    uint24_t currentTime = systick_read();

    while (systick_read() < currentTime + mseconds);
}