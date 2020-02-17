#include "ui_submenus.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/system_time.h"
#include "rf_sensor.h"
#include "ui.h"
#include "ui_idle_block.h"

/* ************************************************************************** */
#define SUBMENU_DURATION 2200

void scale_submenu(void) {
    blink_scale(3); // blink for emphasis
    show_scale();   // leave it on the screen

    system_time_t startTime = get_current_time(); // stash the current time
    while (1) {
        if (btn_is_down(LUP)) {
            startTime = get_current_time(); // reset the start time
            toggle_scale();
            blink_scale(2); // blink for emphasis
            show_scale();   // leave it on the screen
        }

        if (btn_is_down(FUNC)) {
            break; // Pressing FUNC again cancels and exits
        }
        if (time_since(startTime) >= SUBMENU_DURATION) {
            break; // Cancel and exit if it's been longer than SUBMENU_DURATION
        }
        if (RF_is_present()) {
            return; // Immediately exit if RF is detected
        }
        ui_idle_block();
    }
    blink_scale(4);
}

void auto_submenu(void) {
    blink_auto(3); // blink for emphasis
    show_auto();   // leave it on the screen

    system_time_t startTime = get_current_time(); // stash the current time
    while (1) {
        if (btn_is_down(CDN)) {
            startTime = get_current_time(); // reset the start time
            toggle_auto();
            blink_auto(2); // blink for emphasis
            show_auto();   // leave it on the screen
        }

        if (btn_is_down(FUNC)) {
            break; // Pressing FUNC again cancels and exits
        }
        if (time_since(startTime) >= SUBMENU_DURATION) {
            break; // Cancel and exit if it's been longer than SUBMENU_DURATION
        }
        if (RF_is_present()) {
            return; // Immediately exit if RF is detected
        }
        ui_idle_block();
    }
    blink_auto(4);
}

void threshold_submenu(void) {
    blink_thresh(3); // blink for emphasis
    show_thresh();   // leave it on the screen

    system_time_t startTime = get_current_time(); // stash the current time
    while (1) {
        if (btn_is_down(LDN)) {
            startTime = get_current_time(); // reset the start time
            SWR_threshold_increment();
            blink_thresh(2); // blink for emphasis
            show_thresh();   // leave it on the screen
        }

        if (btn_is_down(FUNC)) {
            break; // Pressing FUNC again cancels and exits
        }
        if (time_since(startTime) >= SUBMENU_DURATION) {
            break; // Cancel and exit if it's been longer than SUBMENU_DURATION
        }
        if (RF_is_present()) {
            return; // Immediately exit if RF is detected
        }
        ui_idle_block();
    }
    blink_thresh(4);
}

/* -------------------------------------------------------------------------- */

void function_submenu(void) {
    while (btn_is_down(FUNC)) {
        ui_idle_block();
    }

    delay_ms(50);
    play_interruptable_animation(&arrow_up[0]);
    display_single_frame(&arrow_up[0], 1);

    system_time_t startTime = get_current_time(); // stash the current time

    while (1) {
        if (btn_is_down(CUP)) {
            toggle_peak();
            show_peak();
            return;
        }
        if (btn_is_down(LUP)) {
            scale_submenu();
            return;
        }
        if (btn_is_down(CDN)) {
            auto_submenu();
            return;
        }
        if (btn_is_down(LDN)) {
            threshold_submenu();
            return;
        }
        if (btn_is_down(ANT)) {
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if (btn_is_down(TUNE)) {
            manual_store();
            while (btn_is_down(TUNE)) {
                ui_idle_block();
            }
            return;
        }

        // Pressing FUNC cancels and exits
        if (btn_is_down(FUNC)) {
            break;
        }

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (time_since(startTime) >= SUBMENU_DURATION) {
            break;
        }

        // Immediately exit if RF is detected
        if (RF_is_present()) {
            return;
        }

        ui_idle_block();
    }
    while (btn_is_down(FUNC)) {
        ui_idle_block();
    }
    play_animation(&arrow_down[0]);
}
