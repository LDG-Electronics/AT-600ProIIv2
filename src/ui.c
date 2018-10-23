#include "ui.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/log_macros.h"
#include "os/shell/shell.h"
#include "os/system_time.h"
#include "relays.h"
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

// TODO: idle block activities need to be profiled for average/maximum runtime
// TODO: need round-robin idle task dispatcher?
void ui_idle_block(void) {
    // RF_sensor_update();
    // update_front_panel();
    shell_update();
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

#define RELAY_NO_CHANGE 0
#define RELAY_INCDEC_SUCCESSFUL 1
#define RELAY_INCDEC_FAILURE -1

#define BLINK_INTERVAL 175
void play_relay_animation(int8_t capResult, int8_t indResult) {
    static system_time_t previousBlinkTime = 0;
    static uint8_t blinkState = 0xff;
    static bool blinkingUpperBar = false;
    static bool blinkingLowerBar = false;
    static uint8_t upperBarBlinkCount = 0;
    static uint8_t lowerBarBlinkCount = 0;

    /*  figure out whether each bar is currently blinking
        xxxResult values:
        1   show current relays
        0   continue previous blink or do nothing
        -1  enable blinking
    */
    if (capResult == 1) {
        blinkingLowerBar = false;
    } else if (capResult == -1) {
        blinkingLowerBar = true;
        lowerBarBlinkCount = 0;
    }
    if (indResult == 1) {
        blinkingUpperBar = false;
    } else if (indResult == -1) {
        blinkingUpperBar = true;
        upperBarBlinkCount = 0;
    }

    if (blinkingUpperBar) {
        displayBuffer.next.upper = blinkState;
    } else {
        displayBuffer.next.upper = currentRelays[system_flags.antenna].inds;
    }
    if (blinkingLowerBar) {
        displayBuffer.next.lower = blinkState;
    } else {
        displayBuffer.next.lower = currentRelays[system_flags.antenna].caps;
    }

    if (systick_elapsed_time(previousBlinkTime) >= BLINK_INTERVAL) {
        previousBlinkTime = systick_read();

        if (blinkingUpperBar) {
            upperBarBlinkCount++;
        }
        if (upperBarBlinkCount == 6) {
            blinkingUpperBar = false;
        }
        if (blinkingLowerBar) {
            lowerBarBlinkCount++;
        }
        if (lowerBarBlinkCount == 6) {
            blinkingLowerBar = false;
        }

        // invert the state for the next iteration
        blinkState = ~blinkState;
    }

    // publish whatever we decided we needed
    push_frame_buffer();
}

uint8_t calculate_retrigger_delay(uint8_t incrementCount) {
    if (incrementCount < 8) {
        return 200;
    } else if (incrementCount < 26) {
        return 100;
    } else {
        return 75;
    }
}

#define TIMEOUT_INTERVAL 1000
void relay_button_hold(void) {
    LOG_TRACE({ println("relay_button_hold"); });
    system_time_t currentTime = systick_read();
    system_time_t lastTriggerTime = currentTime;
    uint8_t triggerCount = 0;
    uint8_t retriggerDelay = 0;
    relays_s newRelays = currentRelays[system_flags.antenna];

    int8_t capResult = 0;
    int8_t indResult = 0;

    // stay in loop while any relay button is held
    while (1) {
        if (check_multiple_buttons(&btn_is_down, 4, CUP, CDN, LUP, LDN)) {
            currentTime = systick_read();
        }
        if (systick_elapsed_time(currentTime) >= TIMEOUT_INTERVAL) {
            break;
        }
        if (systick_elapsed_time(lastTriggerTime) >= retriggerDelay) {
            lastTriggerTime = systick_read();
            // capacitor buttons
            capResult = 0;
            if (btn_is_down(CUP) && btn_is_down(CDN)) {
                // can't go up and down at the same time; do nothing
            } else if (btn_is_down(CUP)) {
                capResult = -1;
                if (newRelays.caps < MAX_CAPACITORS) {
                    newRelays.caps++;
                    capResult = 1;
                }
            } else if (btn_is_down(CDN)) {
                capResult = -1;
                if (newRelays.caps > MIN_CAPACITORS) {
                    newRelays.caps--;
                    capResult = 1;
                }
            }
            // inductor buttons
            indResult = 0;
            if (btn_is_down(LUP) && btn_is_down(LDN)) {
                // can't go up and down at the same time; do nothing
            } else if (btn_is_down(LUP)) {
                indResult = -1;
                if (newRelays.inds < MAX_INDUCTORS) {
                    newRelays.inds++;
                    indResult = 1;
                }
            } else if (btn_is_down(LDN)) {
                indResult = -1;
                if (newRelays.inds > MIN_INDUCTORS) {
                    newRelays.inds--;
                    indResult = 1;
                }
            }
            put_relays(&newRelays);

            // retrigger timing stuff
            if (triggerCount < UINT8_MAX) {
                triggerCount++;
            }
            retriggerDelay = calculate_retrigger_delay(triggerCount);
        }
        play_relay_animation(capResult, indResult);
        ui_idle_block();
    }
    display_clear();
}

/* -------------------------------------------------------------------------- */
/*  Handler for holding TUNE button

    TUNE button behavior is determined by how long you hold the button for.

    A short press toggles between bypass and not-bypass.
    A medium press initiates a memory tune.
    A long press initiates a full tune.
    Holding the button for too long cancels out and does nothing. This is help
    mitigate accidental button presses.

    It plays an animation on the bargraph to represent how long the button has
    been held for:

    Short press:
    |*------*|
    |*------*|
    Medium press:
    |--*--*--|
    |--*--*--|
    Long press:
    |---**---|
    |---**---|
*/
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

/* -------------------------------------------------------------------------- */
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
    LOG_TRACE({ println("func_hold"); });
    bool pressedOtherButton = false;

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

        ui_idle_block();
    }
    if (!pressedOtherButton) {
        function_submenu();
    }

    display_clear();
}

/* -------------------------------------------------------------------------- */
/*  Handler for holding ANT button

*/
void ant_hold(void) {
    LOG_TRACE({ println("ant_hold"); });
    toggle_antenna();
    blink_antenna();
    update_antenna_LED();

    while (btn_is_down(ANT)) {
        ui_idle_block();
    }
}

/* -------------------------------------------------------------------------- */
/*  Handler for holding POWER button

*/
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