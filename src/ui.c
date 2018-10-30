#include "ui.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "frequency_counter.h"
#include "os/buttons.h"
#include "os/log_macros.h"
#include "os/shell/shell.h"
#include "os/system_time.h"
#include "relays.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_TRACE;

/* ************************************************************************** */

typedef struct {
    unsigned RFisPresent : 1;
    unsigned updatingBargraphs : 1;
} ui_flags_t;

ui_flags_t uiFlags;

/* ************************************************************************** */
/*  Notes on the system idle block
    This function contains various 'background' activities that should be
    periodically serviced when the system isn't doing anything else important.
*/

#define FREQUENCY_SAMPLE_INTERVAL 1000
#define RF_SAMPLE_INTERVAL 50
#define BARGRAPH_UPDATE_INTERVAL 30
void ui_idle_block(void) {
    uiFlags.RFisPresent = check_for_RF();

    if (uiFlags.RFisPresent) {
        // measure frequency
        if (systick_elapsed_time(currentRF.lastFrequencyTime) >
            FREQUENCY_SAMPLE_INTERVAL) {
            LOG_TRACE({ println("updating frequency"); });
            get_frequency();
        }

        // then measure RF
        if (systick_elapsed_time(currentRF.lastRFTime) > RF_SAMPLE_INTERVAL) {
            LOG_TRACE({ println("updating RF"); });
            measure_RF();
        }
        // then update bargraphs
        if (uiFlags.updatingBargraphs) {
            static system_time_t lastUpdateTime = 0;
            if (systick_elapsed_time(lastUpdateTime) >
                BARGRAPH_UPDATE_INTERVAL) {
                LOG_TRACE({ println("updating bargraphs"); });
                show_current_power_and_SWR();
            }
            lastUpdateTime = systick_read();
        }
    } else {
        // display_clear();
    }

    shell_update();
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

/* ************************************************************************** */
/*  Top level ui loops

*/
/* ************************************************************************** */
/*  Collective handler for CUP, CDN, LUP, and LDN

*/
typedef enum {
    RLY_INC_NO_CHANGE = 0,     // continue previous blink or do nothing
    RLY_INCREMENT_SUCCESS = 1, // show current relays
    RLY_LIMIT_REACHED = -1,    // begin/continue blinking
} relayIncrementResult_t;

// WARNING - Dragons incoming
#define BLINK_INTERVAL 100
static display_frame_t relay_animation_handler(int8_t capResult,
                                               int8_t indResult) {
    static system_time_t previousBlinkTime = 0;
    static uint8_t blinkFrame = 0xff;
    static bool upperBarIsBlinking = false;
    static bool lowerBarIsBlinking = false;
    static uint8_t upperBarBlinkCount = 0;
    static uint8_t lowerBarBlinkCount = 0;

    /*  When we transition from !blinking -> blinking, reset previousBlinkTime
        and blinkFrame

        This prevents the blinking from starting in the middle of a cycle.
        It also prevents the blinking from starting on an 'off' cycle.
    */
    if ((!lowerBarIsBlinking && !upperBarIsBlinking) &&
        ((capResult == RLY_LIMIT_REACHED) ||
         (indResult == RLY_LIMIT_REACHED))) {
        previousBlinkTime = systick_read();
        blinkFrame = 0xff;
    }

    // to blink or not to blink
    if (capResult == RLY_INCREMENT_SUCCESS) {
        lowerBarIsBlinking = false;
    } else if (capResult == RLY_LIMIT_REACHED) {
        lowerBarIsBlinking = true;
        lowerBarBlinkCount = 0;
    }
    if (indResult == RLY_INCREMENT_SUCCESS) {
        upperBarIsBlinking = false;
    } else if (indResult == RLY_LIMIT_REACHED) {
        upperBarIsBlinking = true;
        upperBarBlinkCount = 0;
    }

    // load the appropriate images into the frame buffer
    relays_t relays = read_current_relays();
    display_frame_t newFrame;
    newFrame.upper = relays.inds;
    newFrame.lower = relays.caps;

    // is either bar blinking?
    if (upperBarIsBlinking || lowerBarIsBlinking) {
        if (upperBarIsBlinking) {
            newFrame.upper = blinkFrame;
        }
        if (lowerBarIsBlinking) {
            newFrame.lower = blinkFrame;
        }

        // count time between blinks, and count number of blinks
        if (systick_elapsed_time(previousBlinkTime) >= BLINK_INTERVAL) {
            previousBlinkTime = systick_read();

            if (upperBarIsBlinking) {
                upperBarBlinkCount++;
            }
            if (upperBarBlinkCount == 6) {
                upperBarIsBlinking = false;
            }
            if (lowerBarIsBlinking) {
                lowerBarBlinkCount++;
            }
            if (lowerBarBlinkCount == 6) {
                lowerBarIsBlinking = false;
            }

            // invert the frame in preparation for the next iteration
            blinkFrame = ~blinkFrame;
        }
    }

    return newFrame;
}

void process_CUP_or_CDN(relays_t *relays, int8_t *capResult) {
    *capResult = RLY_INC_NO_CHANGE;

    if (btn_is_down(CUP) && btn_is_down(CDN)) {
        // can't go up and down at the same time; do nothing
    } else if (btn_is_down(CUP)) {
        if (relays->caps < MAX_CAPACITORS) {
            relays->caps++;
            *capResult = RLY_INCREMENT_SUCCESS;
        } else {
            *capResult = RLY_LIMIT_REACHED;
        }
    } else if (btn_is_down(CDN)) {
        if (relays->caps > MIN_CAPACITORS) {
            relays->caps--;
            *capResult = RLY_INCREMENT_SUCCESS;
        } else {
            *capResult = RLY_LIMIT_REACHED;
        }
    }
}

void process_LUP_or_LDN(relays_t *relays, int8_t *indResult) {
    *indResult = RLY_INC_NO_CHANGE;

    if (btn_is_down(LUP) && btn_is_down(LDN)) {
        // can't go up and down at the same time; do nothing
    } else if (btn_is_down(LUP)) {
        if (relays->inds < MAX_INDUCTORS) {
            relays->inds++;
            *indResult = RLY_INCREMENT_SUCCESS;
        } else {
            *indResult = RLY_LIMIT_REACHED;
        }
    } else if (btn_is_down(LDN)) {
        if (relays->inds > MIN_INDUCTORS) {
            relays->inds--;
            *indResult = RLY_INCREMENT_SUCCESS;
        } else {
            *indResult = RLY_LIMIT_REACHED;
        }
    }
}

#define TIMEOUT_INTERVAL 500
static int8_t timeout_handler(void) {
    // this keeps track of the last time a relay button was down
    static system_time_t lastTimeButtonWasDown;

    if (btn_is_down(CUP) || btn_is_down(CDN) || btn_is_down(LUP) ||
        btn_is_down(LDN)) {
        lastTimeButtonWasDown = systick_read();
    } else {
        // if we ARE NOT holding a relay button, any other button press should
        // kick us out
        if (btn_is_down(POWER) || btn_is_down(ANT) || btn_is_down(FUNC) ||
            btn_is_down(TUNE)) {
            return 0;
        }
    }
    if (systick_elapsed_time(lastTimeButtonWasDown) >= TIMEOUT_INTERVAL) {
        return 0;
    }

    return 1;
}

//
uint8_t calculate_retrigger_delay(uint8_t triggerCount) {
    if (triggerCount < 8) {
        return 200;
    } else if (triggerCount < 32) {
        return 100;
    } else {
        return 75;
    }
}

/*  relay_button_hold() is an absolute clusterfuck

    This function can be greatly simplified by using the TuneOS event scheduler.
    Unfortunately, the event scheduler needs more work, and it's not worth it to
    finish it since nothing else in the tuner needs it.

    The reason this is a clusterfuck is because we need multiple, coexisting,
    non-blocking animations that also respond to new user input. This... thing
    relies VERY heavily on the system timer and a bunch of crappy static
    variables to manage animation state through time.

*/
void relay_button_hold(void) {
    LOG_TRACE({ println("relay_button_hold"); });
    system_time_t lastTriggerTime = systick_read();
    uint8_t triggerCount = 0;
    uint8_t retriggerDelay = 0; // trigger immediately the first time
    relays_t relays;

    int8_t capResult = 0;
    int8_t indResult = 0;

    /*  I know the ui loop idiom is not supposed to be while(1), but this is a
        wierd situation. We actually do not want to leave this loop immediately
        when the relay buttons are released. Instead, we need to stay here for
        an extra half second, so that the currently selected relays can linger
        on the display.
    */
    while (timeout_handler()) {
        // only trigger/retrigger if it's been long enough
        if (systick_elapsed_time(lastTriggerTime) >= retriggerDelay) {
            lastTriggerTime = systick_read();

            relays = read_current_relays();
            // increment or decrement as appropriate
            process_CUP_or_CDN(&relays, &capResult);
            process_LUP_or_LDN(&relays, &indResult);
            if ((capResult == RLY_INCREMENT_SUCCESS) ||
                (indResult == RLY_INCREMENT_SUCCESS)) {
                put_relays(&relays);
                // TODO: relay error handling???
            }

            //
            if (triggerCount < UINT8_MAX) {
                triggerCount++;
            }
            retriggerDelay = calculate_retrigger_delay(triggerCount);
        }

        // animations need to be serviced more often than retriggerDelay allows
        display_frame_t frame = relay_animation_handler(capResult, indResult);
        
        // only draw our update if there's no RF
        if (!uiFlags.RFisPresent) {
            displayBuffer.next = frame;

            // publish whatever we decided we needed
            display_update();
        }

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
    uiFlags.updatingBargraphs = 0;
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
        request_memory_tune();
        // request_full_tune();
    } else if (elapsedTime < BTN_PRESS_LONG) {
        request_full_tune();
    } else if (elapsedTime >= BTN_PRESS_LONG) {
        // button was held for too long, do nothing
    }
    uiFlags.updatingBargraphs = 1;
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
    update_status_LEDs();

    while (btn_is_down(ANT)) {
        ui_idle_block();
    }
}

/* -------------------------------------------------------------------------- */
/*  Handler for holding POWER button

*/
// TODO: what do we do with the antenna relay when we power off?
#define POWER_HOLD_DURATION 1500
void power_hold(void) {
    LOG_TRACE({ println("power_hold"); });

    if (systemFlags.powerStatus == 0) {
        set_power_on();
        update_status_LEDs();
        while (btn_is_down(POWER)) {
            ui_idle_block();
        }
    } else {
        system_time_t elapsedTime;
        system_time_t startTime = systick_read();

        while (btn_is_down(POWER)) {
            elapsedTime = systick_elapsed_time(startTime);
            if (elapsedTime >= POWER_HOLD_DURATION) {
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

/* ************************************************************************** */
#define FLAG_SAVE_INTERVAL 200

void ui_mainloop(void) {
    log_register();
    static system_time_t lastFlagSaveTime = 0;
    uiFlags.updatingBargraphs = 1;

    while (1) {
        // Most buttons only work when the system is 'on'
        if (systemFlags.powerStatus == 1) {
            // Relay buttons
            if (btn_is_down(CUP) || btn_is_down(CDN) || btn_is_down(LUP) ||
                btn_is_down(LDN)) {
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
        }

        // POWER works whether the unit is 'on'
        if (btn_is_down(POWER)) {
            power_hold();
        }

        ui_idle_block();

        // save flags
        if (systick_elapsed_time(lastFlagSaveTime) > FLAG_SAVE_INTERVAL) {
            lastFlagSaveTime = systick_read();
            save_flags(); // takes either 80uS or 28mS
        }
    }
}