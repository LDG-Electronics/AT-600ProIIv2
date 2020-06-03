#include "display.h"
#include "flags.h"
#include "os/buttons.h"
#include "os/log_macros.h"
#include "peripherals/pic_header.h"
#include "pins.h"
#include "relays.h"
#include "rf_sensor.h"
#include <float.h>
#include <math.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

double_frame_buffer_t displayBuffer;

/* ************************************************************************** */

void display_init(void) {
    // set bitbang spi display pins to default values
    set_RELAY_CLOCK_PIN(1);
    set_RELAY_DATA_PIN(1);
    set_RELAY_STROBE_PIN(1);

    clear_status_LEDs();

    displayBuffer.next.frame = 0;
    displayBuffer.current.frame = 0;

    log_register();
}

/* -------------------------------------------------------------------------- */
// LED control functions
void clear_status_LEDs(void) {
    set_POWER_LED_PIN(0);
    set_ANT_LED_PIN(0);
    set_BYPASS_LED_PIN(0);
}

void update_status_LEDs(void) {
    if (systemFlags.powerStatus == 1) {
        set_POWER_LED_PIN(systemFlags.powerStatus);
        set_ANT_LED_PIN(~systemFlags.antenna);
        set_BYPASS_LED_PIN(systemFlags.bypassStatus[systemFlags.antenna]);
    } else {
        clear_status_LEDs();
    }
}

/* ************************************************************************** */

static void display_spi_bitbang_tx_word(uint16_t word) {
    set_FP_STROBE_PIN(1);
    set_FP_CLOCK_PIN(0);

    for (uint8_t i = 0; i < 16; i++) {
        if (word & (1 << (15 - i))) {
            set_FP_DATA_PIN(1);
        } else {
            set_FP_DATA_PIN(0);
        }
        delay_us(10);
        set_FP_CLOCK_PIN(1);
        delay_us(10);
        set_FP_CLOCK_PIN(0);
        delay_us(10);
    }
    set_FP_STROBE_PIN(0);
    delay_us(10);
    set_FP_STROBE_PIN(1);
    delay_us(10);
}

// Publish the contents of display.frameBuffer
void push_frame_buffer(void) {
    display_spi_bitbang_tx_word(displayBuffer.next.frame);
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

    if (systemFlags.bypassStatus[systemFlags.antenna] == 1) {
        repeat_animation(&blink_both_bars[0], 3);
    } else {
        displayBuffer.next.upper = relays.inds;
        displayBuffer.next.lower = relays.caps;
        display_update();
        delay_ms(250);
        display_clear();
    }
}

/* -------------------------------------------------------------------------- */

void blink_antenna(void) {
    if (systemFlags.antenna == 1) {
        play_animation(&right_blink[0]);
    } else {
        play_animation(&left_blink[0]);
    }
}

void show_antenna(void) {
    if (systemFlags.antenna == 1) {
        display_single_frame(&right_wave[0], 0);
    } else {
        display_single_frame(&left_wave[0], 0);
    }
}

/* -------------------------------------------------------------------------- */

void blink_auto(uint8_t blinks) {
    repeat_animation(&auto_mode[systemFlags.autoMode][0], blinks);
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

/*  AT-600ProII bargraph has the following markings:

    v- all bars off still needs a value
    0, 10, 25, 50, 100, 200, 300, 450, 600

    v- all bars off still needs a value
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.0+
*/
float fwdIndexArray[10] = {
    0, 10, 25, 50, 100, 200, 300, 450, 600, FLT_MAX,
};

float swrIndexArray[10] = {
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.5, FLT_MAX,
};

// returns the index of the array element whose value is closest to data
uint8_t find_closest_value(float data, float *array) {
    uint8_t lowerNeighbor = 0;
    while (array[lowerNeighbor + 1] < data) {
        lowerNeighbor++;
    }
    uint8_t upperNeighbor = lowerNeighbor + 1;

    float lowerDistance = fabs(data - array[lowerNeighbor]);
    float upperDistance = fabs(array[upperNeighbor] - data);
    uint8_t nearestNeighbor;

    if (lowerDistance < upperDistance) {
        nearestNeighbor = lowerNeighbor;
    } else {
        nearestNeighbor = upperNeighbor;
    }

    if (nearestNeighbor > 9) {
        nearestNeighbor = 9;
    }
    return nearestNeighbor;
}

const uint8_t ledBarTable[9] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};

display_frame_t render_RF(float forwardWatts, float swrValue) {
    display_frame_t frame;

    uint8_t fwdIndex = find_closest_value(forwardWatts, fwdIndexArray);
    frame.upper = ledBarTable[fwdIndex];

    uint8_t swrIndex = find_closest_value(swrValue, swrIndexArray);
    frame.lower = ledBarTable[swrIndex];

    return frame;
}

/* ************************************************************************** */

void print_frame(display_frame_t frame) {
    print("|");
    for (uint8_t i = 0; i < 8; i++) {
        if (frame.upper & (1 << i)) {
            print("*");
        } else {
            print("-");
        }
    }
    print("|\r\n|");
    for (uint8_t i = 0; i < 8; i++) {
        if (frame.lower & (1 << i)) {
            print("*");
        } else {
            print("-");
        }
    }
    println("|");
}