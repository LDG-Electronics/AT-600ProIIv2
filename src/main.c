#include "includes.h"
#include <xc.h>
#include "config.h"
#include "menus.h"

/* ************************************************************************** */

void main(void)
{
    startup();

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
        // if (btn_is_pressed(POWER) || btn_is_down(POWER)) power_hold();
        if (btn_is_pressed(POWER) || btn_is_down(POWER)) ant_hold();
    }
}

