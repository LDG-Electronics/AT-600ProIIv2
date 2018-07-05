#include "includes.h"
#include <xc.h>
#include "peripherals/config.h"

/* ************************************************************************** */

void main(void)
{
    startup();

    while(1)
    {
        // Relay buttons
        if (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
            relay_button_hold();
        }

        // Other buttons
        if (btn_is_down(FUNC)) func_hold();
        if (btn_is_down(TUNE)) tune_hold();
        if (btn_is_down(ANT)) ant_hold();
        if (btn_is_down(POWER)) power_hold();

        system_idle_block();
    }
}