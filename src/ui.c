#include "ui.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/event_scheduler.h"
#include "os/log_macros.h"
#include "os/shell/shell.h"
#include "os/system_time.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

#define FRONT_PANEL_UPDATE_INTERVAL 30
void update_front_panel(void) {
    static system_time_t lastUpdateTime = 0;

    // TODO: this is fucking up the background animation system
    // Do the existing display mutexes solve this?

    if (systick_elapsed_time(lastUpdateTime) > FRONT_PANEL_UPDATE_INTERVAL) {
        show_current_power_and_SWR();
    }

    lastUpdateTime = systick_read();
}

/* ************************************************************************** */
/*  Notes on the system idle block
    This function contains various 'background' activities that should be
    periodically serviced when the system isn't doing anything else important.
*/
void ui_idle_block(void) {
    RF_sensor_update();
    // update_front_panel();
    shell_update();
    event_scheduler_update();
    save_flags();
}

/* ************************************************************************** */

// Button timing constants, in 1ms increments.
#define BTN_PRESS_DEBOUNCE 20
#define BTN_PRESS_SHORT 350
#define BTN_PRESS_MEDIUM 2000
#define BTN_PRESS_LONG 10000

/* ************************************************************************** */
#define SUBMENU_DURATION 2200

void threshold_submenu(void) {
    LOG_TRACE({ println("threshold_submenu"); });
    blink_thresh(3); // blink for emphasis
    show_thresh();   // leave it on the screen

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (btn_is_down(LDN)) {
            startTime = systick_read(); // reset the start time
            SWR_threshold_increment();

            blink_thresh(2); // blink for emphasis
            show_thresh();   // leave it on the screen
        }

        // Pressing FUNC again cancels and exits
        if (btn_is_down(FUNC)) {
            break;
        }

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (systick_elapsed_time(startTime) >= SUBMENU_DURATION) {
            break;
        }

        ui_idle_block();
    }
}

void scale_submenu(void) {
    LOG_TRACE({ println("scale_submenu"); });
    blink_scale(3); // blink for emphasis
    show_scale();   // leave it on the screen

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (btn_is_down(LUP)) {
            startTime = systick_read(); // reset the start time
            
            toggle_scale();

            blink_scale(2); // blink for emphasis
            show_scale();   // leave it on the screen
        }

        // Pressing FUNC again cancels and exits
        if (btn_is_down(FUNC)) {
            break;
        }

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (systick_elapsed_time(startTime) >= SUBMENU_DURATION) {
            break;
        }

        ui_idle_block();
    }
}

void function_submenu(void) {
    LOG_TRACE({ println("function_submenu"); });
    while (btn_is_down(FUNC)) {
        // make sure FUNC is released before we continue
    }

    delay_ms(50);
    play_interruptable_animation(&arrow_up[0]);

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (btn_is_down(CUP)) {
            LOG_TRACE({ println("CUP"); });
            toggle_peak();
            show_peak();
            return;
        }
        if (btn_is_down(LUP)) {
            LOG_TRACE({ println("LUP"); });
            scale_submenu();
            blink_scale(4);
            return;
        }
        if (btn_is_down(CDN)) {
            LOG_TRACE({ println("CDN"); });
            toggle_auto();
            blink_auto(4);
            return;
        }
        if (btn_is_down(LDN)) {
            LOG_TRACE({ println("LDN"); });
            threshold_submenu();
            blink_thresh(4);
            return;
        }
        if (btn_is_down(ANT)) {
            LOG_TRACE({ println("ANT"); });
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if (btn_is_down(TUNE)) {
            LOG_TRACE({ println("TUNE"); });
            manual_store();
            return;
        }

        // Pressing POWER or FUNC cancels and exits
        if (btn_is_down(POWER)) {
            LOG_TRACE({ println("POWER"); });
            break;
        }
        if (btn_is_down(FUNC)) {
            LOG_TRACE({ println("FUNC"); });
            break;
        }

        // Cancel and exit if it's been longer than SUBMENU_DURATION
        if (systick_elapsed_time(startTime) >= SUBMENU_DURATION) {
            LOG_TRACE({ println("function_submenu timeout"); });
            break;
        }

        ui_idle_block();
    }
    play_animation(&arrow_down[0]);
}

#define POWER_BUTTON_DURATION 1000
// TODO: this whole thing needs replaced
void shutdown_submenu(void) {
    LOG_TRACE({ println("shutdown_submenu"); });
    LOG_INFO({ println("shutting down"); });

    // Turn off the whole front panel
    clear_status_LEDs();
    display_clear();

    while (btn_is_down(POWER)) {
        // wait until power button is released
    }
    delay_ms(100);

    system_time_t startTime = systick_read(); // stash the current time

    while (1) {
        if (systick_elapsed_time(startTime) >= POWER_BUTTON_DURATION) {
            break;
        }

        ui_idle_block();
    }

    LOG_INFO({ println("shutting down"); });

    // Put the Status LEDs back how we found them
    update_status_LEDs();

    while (btn_is_down(POWER)) {
        // wait until power button is released
    }
}

/* -------------------------------------------------------------------------- */

void relay_button_hold(void) {
    LOG_TRACE({ println("relay_button_hold"); });
    system_time_t currentTime = systick_read();
    uint8_t incrementCount = 0;
    uint8_t incrementDelay = 200;

    // stay in loop while any relay button is held
    while (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
        if (systick_elapsed_time(currentTime) >= incrementDelay) {
            currentTime = systick_read();

            // can't go up and down at the same time
            if (btn_is_down(CUP) && btn_is_down(CDN)) {
                // do nothing
            } else if (btn_is_down(CUP)) {
                capacitor_increment();
            } else if (btn_is_down(CDN)) {
                capacitor_decrement();
            }
            if (btn_is_down(LUP) && btn_is_down(LDN)) {
                // do nothing
            } else if (btn_is_down(LDN)) {
                inductor_decrement();
            } else if (btn_is_down(LUP)) {
                inductor_increment();
            }

            if (incrementCount < UINT8_MAX) {
                incrementCount++;
            }
            if (incrementCount == 4) {
                incrementDelay = 75;
            }
            if (incrementCount == 32) {
                incrementDelay = 50;
            }
        }
        ui_idle_block();
    }
    // lock_display();
    event_register("display_release", display_release, 1000);
}

void tune_hold(void) {
    LOG_TRACE({ println("tune_hold"); });
    system_time_t elapsedTime;
    system_time_t startTime = systick_read();

    while (btn_is_down(TUNE)) // stay in loop while TUNE is held
    {
        elapsedTime = systick_elapsed_time(startTime);

        if (elapsedTime < BTN_PRESS_DEBOUNCE) {
            // button was not held long enough, do nothing
        } else if (elapsedTime < BTN_PRESS_SHORT) {
            display_single_frame(&center_crawl[0], 0);
        } else if (elapsedTime < BTN_PRESS_MEDIUM) {
            display_single_frame(&center_crawl[0], 2);
        } else if (elapsedTime < BTN_PRESS_LONG) {
            display_single_frame(&center_crawl[0], 3);
        } else if (elapsedTime >= BTN_PRESS_LONG) {
            display_clear();
        }

        ui_idle_block();
    }

    display_clear();

    if (elapsedTime < BTN_PRESS_DEBOUNCE) {
        // button was not held long enough, do nothing
    } else if (elapsedTime < BTN_PRESS_SHORT) {
        toggle_bypass();
        blink_bypass();
    } else if (elapsedTime < BTN_PRESS_MEDIUM) {
        // request_memory_tune();
        request_full_tune();
    } else if (elapsedTime < BTN_PRESS_LONG) {
        request_full_tune();
    } else if (elapsedTime >= BTN_PRESS_LONG) {
        // button was held for too long, do nothing
    }
}

void func_hold(void) {
    LOG_TRACE({ println("func_hold"); });
    uint8_t FuncHoldProcessed = 0;

    while (btn_is_down(FUNC)) // stay in loop while FUNC is held
    {
        if (btn_is_down(CUP)) {
            FuncHoldProcessed = 1;
            show_peak();
        }
        if (btn_is_down(CDN)) {
            FuncHoldProcessed = 1;
            blink_auto(3);
            show_auto();
        }
        if (btn_is_down(LDN)) {
            FuncHoldProcessed = 1;
            blink_thresh(3);
            show_thresh();
        }
        if (btn_is_down(LUP)) {
            FuncHoldProcessed = 1;
            blink_scale(3);
            show_scale();
        }
        if (btn_is_down(ANT)) {
            FuncHoldProcessed = 1;
            blink_HiLoZ(2);
            show_HiLoZ();
        }

        ui_idle_block();
    }
    if (FuncHoldProcessed == 0) {
        function_submenu();
    }
}

void ant_hold(void) {
    LOG_TRACE({ println("ant_hold"); });
    toggle_antenna();
    blink_antenna();
    update_antenna_LED();

    while (btn_is_down(ANT)) {
        ui_idle_block();
    }
}

#define POWER_HOLD_DURATION 1500

void power_hold(void) {
    LOG_TRACE({ println("power_hold"); });
    system_time_t startTime = systick_read();

    while (btn_is_down(POWER)) {
        if (systick_elapsed_time(startTime) >= POWER_HOLD_DURATION) {
            break;
        }

        ui_idle_block();
    }
    delay_ms(25);
}

/* ************************************************************************** */

void ui_mainloop(void) {
    log_register();

    while (1) {
        // Relay buttons
        if (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
            relay_button_hold();
        }

        // Other buttons
        if (btn_is_down(FUNC)) {
            func_hold();
        }
        if (btn_is_down(TUNE)) {
            tune_hold();
        }
        if (btn_is_down(ANT)) {
            ant_hold();
        }
        if (btn_is_down(POWER)) {
            power_hold();
        }

        ui_idle_block();
    }
}