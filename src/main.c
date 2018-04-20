#include "includes.h"
#include <xc.h>
#include "config.h"
#include "buttons.h"
#include "hardware.h"
#include "menus.h"
#include "meter.h"
#include "shell.h"

/* ************************************************************************** */

void main(void)
{    
    startup();

    while(1)
    {
        attempt_meter_update();
        check_for_shell_command();

        // Relay buttons
        if (btn_is_pressed(CUP) || btn_is_down(CUP)) cup_hold();
        if (btn_is_pressed(CDN) || btn_is_down(CDN)) cdn_hold();
        if (btn_is_pressed(LUP) || btn_is_down(LUP)) lup_hold();
        if (btn_is_pressed(LDN) || btn_is_down(LDN)) ldn_hold();

        // Other buttons
        if (btn_is_pressed(FUNC) || btn_is_down(FUNC)) func_hold();
        if (btn_is_pressed(TUNE) || btn_is_down(TUNE)) tune_hold();
        if (btn_is_pressed(ANT) || btn_is_down(ANT)) ant_hold();
        if (btn_is_pressed(POWER) || btn_is_down(POWER)) power_hold();
    }
}

