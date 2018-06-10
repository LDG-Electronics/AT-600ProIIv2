#include "includes.h"
#include <xc.h>
#include "config.h"

/* ************************************************************************** */

void main(void)
{
    startup();

    while(1)
    {
        // Relay buttons
        if (btn_is_pressed(CUP) || btn_is_down(CUP)) relay_button_hold();
        if (btn_is_pressed(CDN) || btn_is_down(CDN)) relay_button_hold();
        if (btn_is_pressed(LUP) || btn_is_down(LUP)) relay_button_hold();
        if (btn_is_pressed(LDN) || btn_is_down(LDN)) relay_button_hold();

        // Other buttons
        if (btn_is_pressed(FUNC) || btn_is_down(FUNC)) func_hold();
        if (btn_is_pressed(TUNE) || btn_is_down(TUNE)) tune_hold();
        if (btn_is_pressed(ANT) || btn_is_down(ANT)) ant_hold();
        if (btn_is_pressed(POWER) || btn_is_down(POWER)) power_hold();

        system_idle_block();
    }
}