#include "includes.h"

/* ************************************************************************** */

void main(void)
{
    startup();


    // while(1)
    // {
    //     print_cat_ln("Freq:", get_freq());
    //     delay_ms(100);
    // }

    while(1)
    {
        // Relay buttons
        if (btn_is_pressed(CUP)) cup_hold();
        if (btn_is_pressed(CDN)) cdn_hold();
        if (btn_is_pressed(LUP)) lup_hold();
        if (btn_is_pressed(LDN)) ldn_hold();
        relays_delay_reset();

        // Other buttons
        if (btn_is_pressed(FUNC)) func_hold();
        if (btn_is_pressed(TUNE)) tune_hold();
        if (btn_is_pressed(POWER)) power_hold();
        if (btn_is_pressed(ANT)) {
            // ANT button is currently broken
            // toggle_ant();
        }
    }
}

