#include "../display.h"
#include "../events.h"
#include "../os/buttons.h"
#include "ui.h"
#include "ui_idle_block.h"
#include "ui_submenus.h"

/* ************************************************************************** */
/*  Handler for holding FUNC button

    The FUNC button is the gateway to changing many of the tuner's settings.

    The other front panel buttons have their functions printed underneath in a
    smaller font.

    Functions:
    POWER: No function
    FUNC: This wouldn't even make sense
    ANT: Hi/LoZ toggle
    CUP: Peak mode toggle
    CDN: Auto mode toggle
    LUP: Scale cycle
    LDN: SWR Threshold cycle
    TUNE: Manually store memory

    Holding the FUNC button + pressing another button shows the current value of
    the related setting. This does not change the setting.

    Pressing and releasing the FUNC button enters "function mode" by calling
    function_submenu(). While in function mode, pressing another button will
    perform the related action and exit function mode.
*/
void func_hold(void) {
    bool pressedOtherButton = false;

    // display SOMETHING to acknowledge the button press
    display_single_frame(&arrow_up[0], 0);

    while (btn_is_down(FUNC)) {
        if (btn_is_down(CUP)) {
            pressedOtherButton = true;
            show_peak();
        }
        if (btn_is_down(CDN)) {
            pressedOtherButton = true;
            blink_auto(3);
            show_auto();
        }
        if (btn_is_down(LDN)) {
            pressedOtherButton = true;
            blink_thresh(3);
            show_thresh();
        }
        if (btn_is_down(LUP)) {
            pressedOtherButton = true;
            blink_scale(3);
            show_scale();
        }
        if (btn_is_down(ANT)) {
            pressedOtherButton = true;
            blink_HiLoZ(2);
            show_HiLoZ();
        }

        // Immediately exit if RF is detected
        if (RF_is_present()) {
            display_clear();
            return;
        }

        ui_idle_block();
    }
    if (!pressedOtherButton) {
        function_submenu();
    }

    display_clear();
}
