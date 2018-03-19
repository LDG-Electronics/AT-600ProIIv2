#include "includes.h"

/* ************************************************************************** */

void main(void)
{
    startup();



    // while(1)
    // {
    //     spi_tx_word(0xffff);
    //     delay_ms(100);
    //     spi_tx_word(0x0000);
    //     delay_ms(100);
    // }
    // while(1)
    // {
    //     display_raw_frame(0xffff);
    //     delay_ms(100);
    //     display_raw_frame(0x0000);
    //     delay_ms(100);
    // }

    while(1)
    {    
        // Relay buttons
        if (btn_is_pressed(CUP) || btn_is_down(CUP)) cup_hold();
        if (btn_is_pressed(CDN) || btn_is_down(CDN)) cdn_hold();
        if (btn_is_pressed(LUP) || btn_is_down(LUP)) lup_hold();
        if (btn_is_pressed(LDN) || btn_is_down(LDN)) ldn_hold();

        // Other buttons
        if (btn_is_pressed(FUNC) || btn_is_down(FUNC)) func_hold();
        if (btn_is_pressed(TUNE) || btn_is_down(TUNE)) tune_hold();

        // ANT button doesn't work in development unit
        // if (btn_is_pressed(ANT) || btn_is_down(ANT)) ant_hold();

        // remap ANT onto POWER
        if (btn_is_pressed(POWER) || btn_is_down(POWER)) power_hold();
        // if (btn_is_pressed(POWER) || btn_is_down(POWER)) ant_hold();
    }
}

