#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/system_time.h"
#include "pins.h"
#include "ui.h"
#include "ui_idle_block.h"

/* ************************************************************************** */
/*  Handler for holding POWER button

*/
#define POWER_HOLD_DURATION 1500
void power_hold(void) {
    if (systemFlags.powerStatus == 0) {
        if (set_power_on()) {
            update_status_LEDs();
            play_animation(&right_crawl[0]);
        }

    } else {
        system_time_t startTime = get_current_time();
        while (btn_is_down(POWER)) {
            if (time_since(startTime) >= POWER_HOLD_DURATION) {
                if (set_power_off()) {
                    display_clear();
                    clear_status_LEDs();
                    play_animation(&left_crawl[0]);
                }
                break;
            }

            ui_idle_block();
        }
    }

    while (btn_is_down(POWER)) {
        ui_idle_block();
    }
}