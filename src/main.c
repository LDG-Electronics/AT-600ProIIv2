#include "includes.h"

/* ************************************************************************** */

void main(void)
{
    // double test = 0.5;
    // char buffer[17];

    startup();

    // sprintf(&buffer, "%f", test);

    // log_str_ln("hello!");
    // log_str_ln(&buffer);

    // while(1)
    // {
    //     play_animation(&center_crawl);

    //     delay_ms(500);
    // }

    while(1)
    {
        // take_SWR_samples();

        save_flags();
    
        // Relay buttons
        if (btn_is_pressed(CUP)) cup_hold();
        if (btn_is_pressed(CDN)) cdn_hold();
        if (btn_is_pressed(LUP)) lup_hold();
        if (btn_is_pressed(LDN)) ldn_hold();
        relays_delay_reset();

        // Other buttons
        if (btn_is_pressed(FUNC)) func_hold();
        if (btn_is_pressed(TUNE)) tune_hold();

        // ANT button doesn't work in development unit
        // if (btn_is_pressed(ANT)) ant_hold();

        // remap ANT onto POWER
        // if (btn_is_pressed(POWER)) power_hold();
        if (btn_is_pressed(POWER)) ant_hold();
    }
}

