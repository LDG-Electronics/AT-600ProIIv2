#include "includes.h"

#include "os/buttons.h"
#include "peripherals/pins.h"
#include "peripherals/spi.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

const uint8_t swrThreshDisplay[] = {0x08, 0x10, 0x20, 0x40};
const uint8_t ledBarTable[] = {
    0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff,
};

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
    uint8_t i = 0;
    bool useUpper = true;
    bool useLower = true;

    // Check if the provided animation has a header frame
    if (animation[0].frame_delay == NULL) {
        useUpper = animation[0].upper;
        useLower = animation[0].lower;

        // first frame was metadata, animation begins on second frame
        i = 1;
    }

    while (1) {
        if (useUpper)
            displayBuffer.next.upper = animation[i].upper;
        if (useLower)
            displayBuffer.next.lower = animation[i].lower;
        push_frame_buffer();

        if (animation[i].frame_delay == NULL)
            break;
        delay_ms(animation[i].frame_delay);

        i++;
    }
}

// Play an animation from animations.h, and repeat it n times
void repeat_animation(const animation_s *animation, uint8_t repeats) {
    for (uint8_t i = 0; i < repeats; i++) {
        play_animation(animation);
    }
}

// Play an animation from animations.h and return early if a button is pressed
void play_interruptable_animation(const animation_s *animation) {
    uint8_t i = 0;
    uint16_t j = 0;
    bool useUpper = true;
    bool useLower = true;

    // Check if the provided animation has a header frame
    if (animation[i].frame_delay == NULL) {
        useUpper = animation[0].upper;
        useLower = animation[0].lower;

        // first frame was metadata, animation begins on second frame
        i++;
    }

    while (1) {
        if (useUpper)
            displayBuffer.next.upper = animation[i].upper;
        if (useLower)
            displayBuffer.next.lower = animation[i].lower;
        push_frame_buffer();

        if (animation[i].frame_delay == NULL)
            break;

        for (j = animation[i].frame_delay; j != 0; j--) {
            if (get_buttons() != 0)
                break;
            delay_ms(1);
        }

        i++;
    }
}

/* -------------------------------------------------------------------------- */

// background animation system, using TuneOS event scheduler

struct {
    const animation_s *animation;
    uint16_t frameIndex;
} bgA; // bgA = backgroundAnimation

void play_animation_in_background(const animation_s *animation) {
    // is the display available?
    if (displayBuffer.upperMutex == 1) {
        // println("upperMutex already claimed");
        return;
    }
    if (displayBuffer.lowerMutex == 1) {
        // println("lowerMutex already claimed");
        return;
    }

    // is there another bgA event in the queue?
    if (event_queue_lookup("bgA") != -1) {
        // println("bgA animation already in event queue");
        return;
    }

    // set up the state variables
    bgA.animation = animation;
    bgA.frameIndex = 0;

    // Check if the provided animation has a header frame
    if (animation[0].frame_delay == NULL) {
        bgA.frameIndex = 1;
    }

    // Register the event
    event_register("bgA", continue_animation_in_background, 1);
}

int16_t continue_animation_in_background(void) {
    bool useUpper = true;
    bool useLower = true;

    // Check if the provided animation has a header frame
    if (bgA.animation[0].frame_delay == NULL) {
        useUpper = bgA.animation[0].upper;
        useLower = bgA.animation[0].lower;
    }

    // publish the current frame
    if (useUpper)
        displayBuffer.next.upper = bgA.animation[bgA.frameIndex].upper;
    if (useLower)
        displayBuffer.next.lower = bgA.animation[bgA.frameIndex].lower;

    push_frame_buffer();

    // a NULL delay is the signal that the animation is completed
    if (bgA.animation[bgA.frameIndex].frame_delay == NULL) {
        // unlock_display();
    }

    return bgA.animation[bgA.frameIndex++].frame_delay;
}

/* -------------------------------------------------------------------------- */
// display functions that use the frame buffer

void show_cap_relays(void) {
    displayBuffer.next.lower = currentRelays[system_flags.antenna].caps;

    push_frame_buffer();
}

void show_ind_relays(void) {
    displayBuffer.next.upper = currentRelays[system_flags.antenna].inds;

    push_frame_buffer();
}

/* -------------------------------------------------------------------------- */

// function-related display functions

void show_peak(void) {
    if (system_flags.peakMode == 0) {
        play_animation(&peak_off);
    } else {
        play_animation(&peak_on);
    }
}

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS
void blink_bypass(void) {
    if (bypassStatus[system_flags.antenna] == 1) {
        repeat_animation(&blink_both_bars, 3);
    } else {
        show_relays();
        delay_ms(150);
        display_clear();
    }
}

void blink_antenna(void) {
    if (system_flags.antenna == 1) {
        play_animation(&right_wave);
    } else {
        play_animation(&left_wave);
    }
}

void blink_auto(uint8_t blinks) {
    if (system_flags.autoMode == 0) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

// TODO: fix this animation
void blink_HiLoZ(uint8_t blinks) {
    if (currentRelays[system_flags.antenna].z == 1) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

void blink_scale(uint8_t blinks) {
    if (system_flags.scaleMode == 0) {
        repeat_animation(&high_scale, blinks);
    } else {
        repeat_animation(&low_scale, blinks);
    }
}

void blink_thresh(uint8_t blinks) {
    uint8_t currentThreshold = swrThreshIndex;
    repeat_animation(&swrThreshold[currentThreshold], blinks);
}

/* -------------------------------------------------------------------------- */
// function-related, single frame display functions
// THESE HAVE NO DELAYS

void show_auto(void) {
    if (system_flags.autoMode == 0) {
        FP_update(0x8181);
    } else {
        FP_update(0x1818);
    }
}

void show_HiLoZ(void) {
    if (currentRelays[system_flags.antenna].z == 1) {
        FP_update(0xc0c0);
    } else {
        FP_update(0x0303);
    }
}

void show_relays(void) {
    display_frame_s frame;

    frame.upper = ((currentRelays[system_flags.antenna].inds & 0x7f) |
                   ((currentRelays[system_flags.antenna].z & 0x01) << 7));
    frame.lower = currentRelays[system_flags.antenna].caps;

    FP_update(frame.frame);
}

void show_scale(void) {
    if (system_flags.scaleMode == 0) {
        FP_update(0x0008);
    } else {
        FP_update(0x0080);
    }
}

void show_thresh(void) {
    display_frame_s frame;
    frame.upper = 0;
    frame.lower = swrThreshDisplay[swrThreshIndex];

    FP_update(frame.frame);
}

/* -------------------------------------------------------------------------- */

/*  fwdIndex

    AT-600ProII PWR bargraph has the following markings:

    0       <- all bars off still needs a value
    10
    25
    50
    100
    200
    300
    450
    600
*/

double fwdIndexArray[] = {
    0, 10, 25, 50, 100, 200, 300, 450, 600,
};

static uint8_t calculate_fwd_index(double forwardWatts) {
    uint8_t i = 0;

    while (fwdIndexArray[i] < forwardWatts) {
        i++;
    }

    return i;
}

/*  swrIndex

    AT-600ProII SWR bargraph has the following markings:

    1.0     <- all bars off still needs a value
    1.1
    1.3
    1.5
    1.7
    2.0
    2.5
    3.0
    3.0+
*/

double swrIndexArray[] = {
    1.0, 1.1, 1.3, 1.5, 1.7, 2.0, 2.5, 3.0, 3.5,
};

static uint8_t calculate_swr_index(double swrValue) {
    uint8_t i = 0;

    while (swrIndexArray[i] < swrValue) {
        i++;
    }

    return i;
}

void show_power_and_SWR(uint16_t forwardWatts, double swrValue) {
    display_frame_s frame;

    frame.upper = ledBarTable[calculate_fwd_index(forwardWatts)];
    frame.lower = ledBarTable[calculate_swr_index(swrValue)];

    FP_update(frame.frame);
}

void show_current_power_and_SWR(void) {
    display_frame_s frame;

    frame.upper = ledBarTable[calculate_fwd_index(currentRF.forwardWatts)];
    frame.lower = ledBarTable[calculate_swr_index(currentRF.swr)];

    FP_update(frame.frame);
}