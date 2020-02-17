#include "ui.h"
#include "display.h"
#include "flags.h"
#include "os/buttons.h"
#include "rf_sensor.h"
#include "ui_bargraphs.h"
#include "ui_idle_block.h"

extern void func_hold(void);
extern void ant_hold(void);
extern void power_hold(void);
extern void relay_button_hold(void);
extern void tune_hold(void);

/* ************************************************************************** */

void ui_mainloop(void) {
    enable_bargraph_updates();
    clear_RF_history();

    while (1) {
        // Most buttons only work when the system is 'on'
        if (systemFlags.powerStatus == 1) {
            // Relay buttons
            if (btn_is_down(CUP) || btn_is_down(CDN) || btn_is_down(LUP) ||
                btn_is_down(LDN)) {
                relay_button_hold();
            }

            // Other buttons
            if (btn_is_down(TUNE)) {
                disable_bargraph_updates();
                tune_hold();
                enable_bargraph_updates();
            }

            if (btn_is_down(FUNC)) {
                if (!RF_is_present()) {
                    disable_bargraph_updates();
                    func_hold();
                    enable_bargraph_updates();
                }
            }

            if (btn_is_down(ANT)) {
                if (!RF_is_present()) {
                    disable_bargraph_updates();
                    ant_hold();
                    enable_bargraph_updates();
                } else {
                    repeat_animation(&toggle_outer_leds[0], 3);
                }
            }
        }

        // POWER works whether the unit is 'on'
        if (btn_is_down(POWER)) {
            if (!RF_is_present()) {
                disable_bargraph_updates();
                power_hold();
                enable_bargraph_updates();
            }
        }

        ui_idle_block();
    }
}