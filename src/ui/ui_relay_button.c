#include "display.h"
#include "events.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/system_time.h"
#include "pins.h"
#include "relays.h"
#include "rf_sensor.h"
#include "ui.h"
#include "ui_idle_block.h"

/* ************************************************************************** */
/*  Collective handler for CUP, CDN, LUP, and LDN

*/
typedef enum {
    RLY_NO_CHANGE = 0,      // continue previous blink or do nothing
    RLY_INCREMENT = 1,      // show current relays
    RLY_DECREMENT = 2,      // show current relays
    RLY_LIMIT_REACHED = -1, // begin/continue blinking
} relayIncrementResult_t;

// WARNING - Dragons incoming
#define BLINK_INTERVAL 100
static display_frame_t relay_animation_handler(int8_t capResult, int8_t indResult) {
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
        ((capResult == RLY_LIMIT_REACHED) || (indResult == RLY_LIMIT_REACHED))) {
        previousBlinkTime = get_current_time();
        blinkFrame = 0xff;
    }

    // to blink or not to blink
    if ((capResult == RLY_INCREMENT) || (capResult == RLY_DECREMENT)) {
        lowerBarIsBlinking = false;
    } else if (capResult == RLY_LIMIT_REACHED) {
        lowerBarIsBlinking = true;
        lowerBarBlinkCount = 0;
    }
    if ((indResult == RLY_INCREMENT) || (indResult == RLY_DECREMENT)) {
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
        if (time_since(previousBlinkTime) >= BLINK_INTERVAL) {
            previousBlinkTime = get_current_time();

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

/* -------------------------------------------------------------------------- */

// checks CUP/CDN/LUP/LDN, returns true if any is down, false if not
// also updates capResult/indResult with the state of CUP/CDN or LUP/LDN
bool check_relay_buttons(relays_t relays, int8_t *capResult, int8_t *indResult) {
    bool buttons = false;

    *capResult = RLY_NO_CHANGE;
    if (btn_is_down(CUP) && btn_is_down(CDN)) {
        buttons = true;
    } else if (btn_is_down(CUP)) {
        buttons = true;
        if (relays.caps < MAX_CAPACITORS) {
            *capResult = RLY_INCREMENT;
        } else {
            *capResult = RLY_LIMIT_REACHED;
        }
    } else if (btn_is_down(CDN)) {
        buttons = true;
        if (relays.caps > MIN_CAPACITORS) {
            *capResult = RLY_DECREMENT;
        } else {
            *capResult = RLY_LIMIT_REACHED;
        }
    }

    *indResult = RLY_NO_CHANGE;
    if (btn_is_down(LUP) && btn_is_down(LDN)) {
        buttons = true;
    } else if (btn_is_down(LUP)) {
        buttons = true;
        if (relays.inds < MAX_INDUCTORS) {
            *indResult = RLY_INCREMENT;
        } else {
            *indResult = RLY_LIMIT_REACHED;
        }
    } else if (btn_is_down(LDN)) {
        buttons = true;
        if (relays.inds > MIN_INDUCTORS) {
            *indResult = RLY_DECREMENT;
        } else {
            *indResult = RLY_LIMIT_REACHED;
        }
    }

    return buttons;
}

// actually do the increment/decrement, if required
void process_results(relays_t *relays, int8_t capResult, int8_t indResult) {
    if (capResult == RLY_INCREMENT) {
        if (relays->caps < MAX_CAPACITORS) {
            relays->caps++;
        }
    } else if (capResult == RLY_DECREMENT) {
        if (relays->caps > MIN_CAPACITORS) {
            relays->caps--;
        }
    }

    if (indResult == RLY_INCREMENT) {
        if (relays->inds < MAX_INDUCTORS) {
            relays->inds++;
        }
    } else if (indResult == RLY_DECREMENT) {
        if (relays->inds > MIN_INDUCTORS) {
            relays->inds--;
        }
    }
}

/* -------------------------------------------------------------------------- */

#define TIMEOUT_INTERVAL 750
static int8_t timeout_handler(void) {
    // this keeps track of the last time a relay button was down
    static system_time_t lastTimeButtonWasDown;

    if (btn_is_down(CUP) || btn_is_down(CDN) || btn_is_down(LUP) || btn_is_down(LDN)) {
        lastTimeButtonWasDown = get_current_time();
    } else {
        // if we ARE NOT holding a relay button, any other button press should
        // kick us out
        if (btn_is_down(POWER) || btn_is_down(ANT) || btn_is_down(FUNC) || btn_is_down(TUNE)) {
            return 0;
        }
    }
    if (time_since(lastTimeButtonWasDown) >= TIMEOUT_INTERVAL) {
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

#define RETRIGGER_RESET_PERIOD 500

void relay_button_hold(void) {
    // used to track button timing
    system_time_t lastTriggerTime = get_current_time();
    uint8_t triggerCount = 0;
    uint8_t retriggerDelay = 0;

    // used to track button state
    int8_t capResult = 0;
    int8_t indResult = 0;

    while (timeout_handler()) {
        // grab the most recently published relays
        relays_t relays = read_current_relays();

        // we need to check buttons more often than retriggerDelay allows
        if (check_relay_buttons(relays, &capResult, &indResult)) {
            // make sure we don't update the relays too ofter
            if (time_since(lastTriggerTime) >= retriggerDelay) {
                lastTriggerTime = get_current_time();

                // increment or decrement relays as necessary
                process_results(&relays, capResult, indResult);

                // push our relays out to the hardware
                if (put_relays(relays) == -1) {
                    repeat_animation(&toggle_inner_leds[0], 4);
                }

                if (triggerCount < UINT8_MAX) {
                    triggerCount++;
                }
                retriggerDelay = calculate_retrigger_delay(triggerCount);
            }
        } else {
            // retrigger immediately if button is released
            retriggerDelay = 0;

            // reset retrigger if it hasn't begun accelerating
            if (triggerCount < 8) {
                triggerCount = 0;
            }

            // reset retrigger acceleration if we're idle for too long
            if (time_since(lastTriggerTime) >= RETRIGGER_RESET_PERIOD) {
                triggerCount = 0;
            }
        }

        // animations need to be serviced more often than retriggerDelay allows
        display_frame_t frame = relay_animation_handler(capResult, indResult);

        // only draw our update if there's no RF
        if (RF_is_absent()) {
            displayBuffer.next = frame;

            // publish whatever we decided we needed
            display_update();
        }

        // disable autotuning every time we call ui_idle_block()
        disable_auto_tuning();

        ui_idle_block();
    }
    display_clear();
}