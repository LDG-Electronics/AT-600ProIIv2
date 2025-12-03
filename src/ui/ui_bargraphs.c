#include "ui_bargraphs.h"
#include "display.h"
#include "flags.h"
#include "os/system_time.h"
#include "rf_sensor.h"
#include "ui.h"
#include "ui_idle_block.h"
#include <stdbool.h>

/* ************************************************************************** */
/*  allowedToUpdate controls whether the current RF should be displayed on the
    front panel.

    if RF is present:
    we need to display it
    FUNC should be disabled (//? Is this true?)
    POWER should be disabled
    ANT should be disabled
    CUP/CDN/LUP/LDN should execute no matter what, but...
    CUP/CDN/LUP/LDN should not use the display

    bargraph updates are disabled when:
    already inside tune_hold(), func_hold(), ant_hold(), or power_hold()
*/
bool allowedToUpdate = true;

void enable_bargraph_updates(void) { allowedToUpdate = true; }
void disable_bargraph_updates(void) { allowedToUpdate = false; }

/* -------------------------------------------------------------------------- */

uint16_t get_decay_cooldown(uint8_t decayCount) {
    if (decayCount < 2) {
        return 300;
    } else if (decayCount < 4) {
        return 150;
    } else {
        return 50;
    }
}

static display_frame_t prevFrame;

display_frame_t attempt_peak_decay(display_frame_t newFrame) {
    static uint8_t decayCount = 0;
    if (systemFlags.peakMode) {
        static system_time_t lastFrameDecay = 0;
        if (time_since(lastFrameDecay) > get_decay_cooldown(decayCount)) {
            lastFrameDecay = get_current_time();

            decayCount++;

            // shift all the pixels left one space
            prevFrame.upper >>= 1;
            prevFrame.lower >>= 1;
        }
        // Combine the old frame with the new frame
        newFrame.upper |= prevFrame.upper;
        newFrame.lower |= prevFrame.lower;
    }

    // don't let the decay accelerate if the radio is keyed
    if (RF_is_present()) {
        decayCount = 0;
    }

    return newFrame;
}

/* -------------------------------------------------------------------------- */

#define OVERSCALE_BLINK_COOLDOWN 250
void invert_frame(uint8_t *blinkFrame) {
    static system_time_t previousBlinkTime = 0;
    // count time between blinks, and count number of blinks
    if (time_since(previousBlinkTime) >= OVERSCALE_BLINK_COOLDOWN) {
        previousBlinkTime = get_current_time();
        // invert the frame in preparation for the next iteration
        *blinkFrame = ~*blinkFrame;
    }
}

const float overscale[2] = {25, 250};

display_frame_t attempt_overscale_blink(display_frame_t newFrame) {
    if (currentRF.forwardWatts > overscale[systemFlags.scaleMode]) {
        static uint8_t blinkFrame = 0x50;

        // overwrite upper bar with overscale blink frame
        newFrame.upper = 0x0f;
        newFrame.upper |= blinkFrame;

        invert_frame(&blinkFrame);
    }

    return newFrame;
}

/* -------------------------------------------------------------------------- */

void update_display(display_frame_t newFrame) {
    static bool needToClearDisplay = false;
    if (RF_is_present() || (newFrame.frame != 0)) {
        if (allowedToUpdate) {
            needToClearDisplay = true;
            // copy our frame to the frame buffer and push it to the display
            displayBuffer.next = newFrame;
            display_update();
            // save a copy of the frame for peak mode handler
            prevFrame = newFrame;
        }
    }

    if (RF_is_absent() && (newFrame.frame == 0)) {
        if (needToClearDisplay) {
            needToClearDisplay = false;
            if (allowedToUpdate) {
                display_clear();
            }
        }
    }
}

/* -------------------------------------------------------------------------- */

float get_forward_watts(void) {
    if (systemFlags.scaleMode == 1) {
        return currentRF.forwardWatts; // full scale
    } else {
        return currentRF.forwardWatts * 10; // zoomed scale
    }
}

/* ************************************************************************** */

static bool skipNextPeak = false;

void skip_next_peak_decay(void) {
    skipNextPeak = true; //
}

void update_bargraphs(void) {
    // scale mode handler
    float forwardWatts = get_forward_watts();

    // render the forward power and SWR into a frame
    display_frame_t newFrame = render_RF(forwardWatts, currentRF.swr);

    // peak mode handler
    if (skipNextPeak) {
        skipNextPeak = false;
    } else {
        newFrame = attempt_peak_decay(newFrame);
    }

    // over-scale blink handler
    newFrame = attempt_overscale_blink(newFrame);

    update_display(newFrame);
}