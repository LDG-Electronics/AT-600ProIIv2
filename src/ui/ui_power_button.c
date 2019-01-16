#include "../display.h"
#include "../events.h"
#include "../flags.h"
#include "../os/buttons.h"
#include "../os/system_time.h"
#include "ui.h"
#include "ui_idle_block.h"

/* ************************************************************************** */
/*  Handler for holding POWER button

*/
// TODO: what do we do with the antenna relay when we power off?
#define POWER_HOLD_DURATION 1500
void power_hold(void) {
    if (systemFlags.powerStatus == 0) {
        set_power_on();
        update_status_LEDs();

        while (btn_is_down(POWER)) {
            ui_idle_block();
        }
    } else {
        system_time_t startTime = get_current_time();
        while (btn_is_down(POWER)) {
            if (time_since(startTime) >= POWER_HOLD_DURATION) {
                set_power_off();
                display_clear();
                clear_status_LEDs();

                while (btn_is_down(POWER)) {
                    // make sure we wait here until POWER is released
                }
            }

            ui_idle_block();
        }
    }
}