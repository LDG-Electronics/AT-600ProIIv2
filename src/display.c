#include "display.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/log_macros.h"
#include "peripherals/pic18f46k42.h"
#include "peripherals/pins.h"
#include "peripherals/spi.h"
#include "relays.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

locking_float_buffer_s displayBuffer;

/* ************************************************************************** */

void display_init(void) {
    spi_init();

    clear_status_LEDs();

    displayBuffer.next.frame = 0;
    displayBuffer.current.frame = 0;
    displayBuffer.upperMutex = 0;
    displayBuffer.lowerMutex = 0;

    log_register();
}

/* -------------------------------------------------------------------------- */
// LED control functions
void clear_status_LEDs(void) {
    POWER_LED_PIN = 0;
    ANT_LED_PIN = 0;
    BYPASS_LED_PIN = 0;
}

void update_status_LEDs(void) {
    if (systemFlags.powerStatus == 1) {
        POWER_LED_PIN = systemFlags.powerStatus;
        ANT_LED_PIN = ~systemFlags.antenna;
        BYPASS_LED_PIN = bypassStatus[systemFlags.antenna];
    } else {
        clear_status_LEDs();
    }
}

/* ************************************************************************** */

// Publish the contents of display.frameBuffer
void push_frame_buffer(void) {
    spi_tx_word(displayBuffer.next.frame);
    displayBuffer.current.frame = displayBuffer.next.frame;
}

void display_update(void) {
    if (systemFlags.powerStatus == 1) {
        push_frame_buffer();
    } else {
        // if the unit is 'off', make sure we can't accidentally display stuff
        displayBuffer.next.frame = 0;
        push_frame_buffer();
    }
}

// Clears the display by turning off both bargraphs
void display_clear(void) {
    displayBuffer.next.frame = 0;
    push_frame_buffer();
}

// Clears the display and releases the display object
int16_t display_release(void) {
    display_clear();

    // unlock_display();
    return 0;
}

// Display a single from an animation
void display_single_frame(const animation_s *animation, uint8_t frame_number) {
    displayBuffer.next.upper = animation[frame_number].upper;
    displayBuffer.next.lower = animation[frame_number].lower;
    push_frame_buffer();
}

// Play an animation from animations.h
void play_animation(const animation_s *animation) {
    LOG_TRACE({ println("play_animation"); });
    uint8_t i = 0;
    bool useUpper = true;
    bool useLower = true;

    // Check if the provided animation has a header frame
    if (animation[0].frame_delay == 0) {
        useUpper = animation[0].upper;
        useLower = animation[0].lower;

        // first frame was metadata, animation begins on second frame
        i = 1;
    }

    while (1) {
        if (useUpper) {
            displayBuffer.next.upper = animation[i].upper;
        }
        if (useLower) {
            displayBuffer.next.lower = animation[i].lower;
        }
        push_frame_buffer();

        if (animation[i].frame_delay == 0) {
            break;
        }
        delay_ms(animation[i].frame_delay);

        i++;
    }
}

// Play an animation from animations.h, and repeat it n times
void repeat_animation(const animation_s *animation, uint8_t repeats) {
    LOG_TRACE({ println("repeat_animation"); });
    while (repeats--) {
        play_animation(animation);
    }
}

// Play an animation from animations.h and return early if a button is pressed
void play_interruptable_animation(const animation_s *animation) {
    LOG_TRACE({ println("play_interruptable_animation"); });
    uint8_t i = 0;
    uint16_t j = 0;
    bool useUpper = true;
    bool useLower = true;

    // Check if the provided animation has a header frame
    if (animation[i].frame_delay == 0) {
        useUpper = animation[0].upper;
        useLower = animation[0].lower;

        // first frame was metadata, animation begins on second frame
        i++;
    }

    while (1) {
        if (useUpper) {
            displayBuffer.next.upper = animation[i].upper;
        }
        if (useLower) {
            displayBuffer.next.lower = animation[i].lower;
        }
        push_frame_buffer();

        if (animation[i].frame_delay == 0) {
            break;
        }

        for (j = animation[i].frame_delay; j != 0; j--) {
            if (get_buttons() != 0) {
                break;
            }
            delay_ms(1);
        }

        i++;
    }
}

/* -------------------------------------------------------------------------- */

void show_peak(void) {
    if (systemFlags.peakMode == 0) {
        play_animation(&peak_off[0]);
    } else {
        play_animation(&peak_on[0]);
    }
}

void blink_bypass(void) {
    relays_t relays = read_current_relays();

    if (bypassStatus[systemFlags.antenna] == 1) {
        repeat_animation(&blink_both_bars[0], 3);
    } else {
        displayBuffer.next.upper = relays.inds;
        displayBuffer.next.lower = relays.caps;
        display_update();
        delay_ms(250);
        display_clear();
    }
}

void blink_antenna(void) {
    if (systemFlags.antenna == 1) {
        play_animation(&right_wave[0]);
    } else {
        play_animation(&left_wave[0]);
    }
}

/* -------------------------------------------------------------------------- */

void blink_auto(uint8_t blinks) {
    if (systemFlags.autoMode == 0) {
        repeat_animation(&auto_off[0], blinks);
    } else {
        repeat_animation(&auto_on[0], blinks);
    }
}

void show_auto(void) {
    if (systemFlags.autoMode == 0) {
        displayBuffer.next.lower = 0x81;
        displayBuffer.next.upper = 0x81;
    } else {
        displayBuffer.next.lower = 0x18;
        displayBuffer.next.upper = 0x18;
    }

    display_update();
}

/* -------------------------------------------------------------------------- */

void blink_HiLoZ(uint8_t blinks) {
    if (currentRelays[systemFlags.antenna].z == 1) {
        repeat_animation(&hiz_wave[0], blinks);
    } else {
        repeat_animation(&loz_wave[0], blinks);
    }
}

void show_HiLoZ(void) {
    if (currentRelays[systemFlags.antenna].z == 1) {
        displayBuffer.next.lower = 0xc0;
        displayBuffer.next.upper = 0xc0;
    } else {
        displayBuffer.next.lower = 0x03;
        displayBuffer.next.upper = 0x03;
    }

    display_update();
}

/* -------------------------------------------------------------------------- */

void blink_scale(uint8_t blinks) {
    if (systemFlags.scaleMode == 0) {
        repeat_animation(&high_scale[0], blinks);
    } else {
        repeat_animation(&low_scale[0], blinks);
    }
}

void show_scale(void) {
    displayBuffer.next.lower = 0;

    if (systemFlags.scaleMode == 0) {
        displayBuffer.next.upper = 0x08;
    } else {
        displayBuffer.next.upper = 0x80;
    }

    display_update();
}

/* -------------------------------------------------------------------------- */

const uint8_t swrThreshDisplay[] = {0x08, 0x10, 0x20, 0x40};

void blink_thresh(uint8_t blinks) {
    repeat_animation(&swrThreshold[swrThreshIndex][0], blinks);
}

void show_thresh(void) {
    displayBuffer.next.upper = 0;
    displayBuffer.next.lower = swrThreshDisplay[swrThreshIndex];

    display_update();
}

/* -------------------------------------------------------------------------- */

const uint8_t ledBarTable[] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};

/*  AT-600ProII bargraph has the following markings:

    v- all bars off still needs a value
    0, 10, 25, 50, 100, 200, 300, 450, 600

    v- all bars off still needs a value
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.0+
*/
float fwdIndexArray[] = {
    0, 10, 25, 50, 100, 200, 300, 450, 600,
};

float swrIndexArray[] = {
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.5,
};

static uint8_t array_lookup(float data, float *array) {
    uint8_t i = 0;

    while (array[i] < data) {
        i++;
    }

    return i;
}

void show_power_and_SWR(uint16_t forwardWatts, float swrValue) {
    displayBuffer.next.upper =
        ledBarTable[array_lookup(forwardWatts, fwdIndexArray)];
    displayBuffer.next.lower =
        ledBarTable[array_lookup(swrValue, swrIndexArray)];

    display_update();
}

// TODO: implement scale and peak modes

void show_current_power_and_SWR(void) {
    if (currentRF.forwardWatts > 0) {
        uint8_t fwdIndex = array_lookup(currentRF.forwardWatts, fwdIndexArray);
        displayBuffer.next.upper = ledBarTable[fwdIndex];
    }

    if (currentRF.swr != 0) {
        uint8_t swrIndex = array_lookup(currentRF.swr, swrIndexArray);
        displayBuffer.next.lower = ledBarTable[swrIndex];
    }

    display_update();
}