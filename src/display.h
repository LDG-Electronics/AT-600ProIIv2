#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* ************************************************************************** */

/*  Notes on the display driver.

    The Power LED, Antenna LED, and Bypass LED are collectively referred to as
    the Status LEDs.
*/

/* ************************************************************************** */

// struct union allowing access to each bargraph without bit shifting
typedef union {
    struct {
        uint8_t upper;
        uint8_t lower;
    };
    uint16_t frame;
} display_frame_s;

// struct that represents the current state of the display object
typedef struct {
    display_frame_s next;    // the frame we're about to push out
    display_frame_s current; // the frame that's currently being displayed
    unsigned upperMutex : 1;
    unsigned lowerMutex : 1;
} locking_double_buffer_s;

extern locking_double_buffer_s displayBuffer;

/* ************************************************************************** */

// Setup
extern void display_init(void);

/* ************************************************************************** */

// Front panel status LED manipulations
extern void clear_status_LEDs(void);
extern void update_status_LEDs(void);
extern void update_antenna_LED(void);
extern void update_bypass_LED(void);
extern void update_power_LED(void);

/* -------------------------------------------------------------------------- */
// Front panel bargraph manipulations

// Publishes a raw frame to the display
extern void FP_update(uint16_t data);

// Publish the contents of display.frameBuffer
extern void push_frame_buffer(void);

// Clears the display by turning off both bargraphs
extern void display_clear(void);

// Clears the display and releases the display object
extern int16_t display_release(void);

// Display a single from an animation
extern void display_single_frame(const animation_s *animation,
                                 uint8_t frame_number);

// Play an animation from animations.h
extern void play_animation(const animation_s *animation);

// Play an animation from animations.h, and repeat it n times
extern void repeat_animation(const animation_s *animation, uint8_t repeats);

// Play an animation from animations.h and return early if a button is pressed
extern void play_interruptable_animation(const animation_s *animation);

// Play an animation in the 'background' using the event scheduler
extern void play_animation_in_background(const animation_s *animation);
extern int16_t continue_animation_in_background(void);

/* -------------------------------------------------------------------------- */

// display functions that use the frame buffer
extern void show_cap_relays(void);
extern void show_ind_relays(void);

/* -------------------------------------------------------------------------- */

extern void show_relays(void);
extern void blink_bypass(void);
extern void blink_antenna(void);
extern void show_peak(void);

// Shows current autoMode
extern void blink_auto(uint8_t blinks);
extern void show_auto(void);

// Shows current HiLoZ setting
extern void blink_HiLoZ(uint8_t blinks);
extern void show_HiLoZ(void);

// Shows current scale
extern void blink_scale(uint8_t blinks);
extern void show_scale(void);

// Shows current SWR Threshold
extern void blink_thresh(uint8_t blinks);
extern void show_thresh(void);

/* -------------------------------------------------------------------------- */

extern void show_power_and_SWR(uint16_t forwardWatts, double swrValue);
extern void show_current_power_and_SWR(void);

#endif