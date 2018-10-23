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

locking_double_buffer_s displayBuffer;

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
    POWER_LED_PIN = system_flags.powerStatus;
    ANT_LED_PIN = ~system_flags.antenna;
    BYPASS_LED_PIN = bypassStatus[system_flags.antenna];
}

void update_antenna_LED(void) { ANT_LED_PIN = ~system_flags.antenna; }

void update_bypass_LED(void) {
    BYPASS_LED_PIN = bypassStatus[system_flags.antenna];
}

void update_power_LED(void) { POWER_LED_PIN = system_flags.powerStatus; }

/* ************************************************************************** */

// Publishes a raw frame to the display
void FP_update(uint16_t data) { spi_tx_word(data); }

// Publish the contents of display.frameBuffer
void push_frame_buffer(void) {
    spi_tx_word(displayBuffer.next.frame);
    displayBuffer.current.frame = displayBuffer.next.frame;
}

// Clears the display by turning off both bargraphs
void display_clear(void) { FP_update(0x0000); }

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
    if (system_flags.peakMode == 0) {
        play_animation(&peak_off[0]);
    } else {
        play_animation(&peak_on[0]);
    }
}

void blink_bypass(void) {
    if (bypassStatus[system_flags.antenna] == 1) {
        repeat_animation(&blink_both_bars[0], 3);
    } else {
        show_relays();
        delay_ms(150);
        display_clear();
    }
}

void blink_antenna(void) {
    if (system_flags.antenna == 1) {
        play_animation(&right_wave[0]);
    } else {
        play_animation(&left_wave[0]);
    }
}

void show_relays(void) {
    relays_t relays = read_current_relays();
    display_frame_t frame;

    frame.upper = ((relays.inds & 0x7f) | ((relays.z & 0x01) << 7));
    frame.lower = relays.caps;

    FP_update(frame.frame);
}

/* -------------------------------------------------------------------------- */

void blink_auto(uint8_t blinks) {
    if (system_flags.autoMode == 0) {
        repeat_animation(&auto_off[0], blinks);
    } else {
        repeat_animation(&auto_on[0], blinks);
    }
}

void show_auto(void) {
    if (system_flags.autoMode == 0) {
        FP_update(0x8181);
    } else {
        FP_update(0x1818);
    }
}

/* -------------------------------------------------------------------------- */

void blink_HiLoZ(uint8_t blinks) {
    if (currentRelays[system_flags.antenna].z == 1) {
        repeat_animation(&hiz_wave[0], blinks);
    } else {
        repeat_animation(&loz_wave[0], blinks);
    }
}

void show_HiLoZ(void) {
    display_frame_t frame;

    if (currentRelays[system_flags.antenna].z == 1) {
        frame.lower = 0xc0;
        frame.upper = 0xc0;
    } else {
        frame.lower = 0x03;
        frame.upper = 0x03;
    }

    FP_update(frame.frame);
}

/* -------------------------------------------------------------------------- */

void blink_scale(uint8_t blinks) {
    if (system_flags.scaleMode == 0) {
        repeat_animation(&high_scale[0], blinks);
    } else {
        repeat_animation(&low_scale[0], blinks);
    }
}

void show_scale(void) {
    display_frame_t frame;
    frame.lower = 0;

    if (system_flags.scaleMode == 0) {
        frame.upper = 0x08;
    } else {
        frame.upper = 0x80;
    }

    FP_update(frame.frame);
}

/* -------------------------------------------------------------------------- */

const uint8_t swrThreshDisplay[] = {0x08, 0x10, 0x20, 0x40};

void blink_thresh(uint8_t blinks) {
    repeat_animation(&swrThreshold[swrThreshIndex][0], blinks);
}

void show_thresh(void) {
    display_frame_t frame;
    frame.upper = 0;

    frame.lower = swrThreshDisplay[swrThreshIndex];

    FP_update(frame.frame);
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
double fwdIndexArray[] = {
    0, 10, 25, 50, 100, 200, 300, 450, 600,
};

double swrIndexArray[] = {
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.5,
};

static uint8_t array_lookup(double data, double *array) {
    uint8_t i = 0;

    while (array[i] < data) {
        i++;
    }

    return i;
}

void show_power_and_SWR(uint16_t forwardWatts, double swrValue) {
    display_frame_t frame;
    frame.frame = 0;

    frame.upper = ledBarTable[array_lookup(forwardWatts, fwdIndexArray)];
    frame.lower = ledBarTable[array_lookup(swrValue, swrIndexArray)];

    FP_update(frame.frame);
}

void show_current_power_and_SWR(void) {
    display_frame_t frame;
    frame.frame = 0;

    if (currentRF.forwardWatts > 0) {
        uint8_t fwdIndex = array_lookup(currentRF.forwardWatts, fwdIndexArray);
        frame.upper = ledBarTable[fwdIndex];
    }

    if (currentRF.swr != 0) {
        uint8_t swrIndex = array_lookup(currentRF.swr, swrIndexArray);
        frame.lower = ledBarTable[swrIndex];
    }

    FP_update(frame.frame);
}