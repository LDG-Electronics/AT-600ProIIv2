#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "animations.h"

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
} display_frame_t;

// struct that represents the current state of the display object
typedef struct {
    display_frame_t next;    // the frame we're about to push out
    display_frame_t current; // the frame that's currently being displayed
} double_frame_buffer_t;

extern double_frame_buffer_t displayBuffer;

/* ************************************************************************** */

// Setup
extern void display_init(void);

/* ************************************************************************** */

// Front panel status LED manipulations
extern void clear_status_LEDs(void);
extern void update_status_LEDs(void);

/* -------------------------------------------------------------------------- */
// Front panel bargraph manipulations

// Publish the contents of display.frameBuffer
extern void display_update(void);

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

/* -------------------------------------------------------------------------- */

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

// returns a frame that shows the provided FWD and SWR
extern display_frame_t render_RF(float forwardWatts, float swrValue);

// returns a frame that shows the current FWD and SWR
extern display_frame_t render_current_RF(void);

#endif // _DISPLAY_H_