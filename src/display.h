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
        uint8_t bottomBar;
        uint8_t topBar;
    };
    uint16_t bothBars;
} display_buffer_s;

// struct that represents the current state of the display object
typedef struct {
    display_buffer_s frameBuffer; // the frame we're about to push out
    display_buffer_s currentFrame; // the frame that's currently being displayed
    unsigned displayIsLocked : 1; // are we im the middle of an animation?
} display_s;

extern display_s display;

#define lock_display() display.displayIsLocked = 1
#define unlock_display() display.displayIsLocked = 0

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
extern void display_release(void);

// Display a single from an animation
extern void display_single_frame(const animation_s *animation, uint8_t frame_number);

// Play an animation from animations.h
extern void play_animation(const animation_s *animation);

// Play an animation from animations.h, and repeat it n times
extern void repeat_animation(const animation_s *animation, uint8_t repeats);

// Play an animation from animations.h and return early if a button is pressed
extern void play_interruptable_animation(const animation_s *animation);

// Background animation stuff
extern void begin_background_animation(const animation_s *animation);
extern void continue_background_animation(void);

/* -------------------------------------------------------------------------- */

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS
extern void blink_bypass(void);
extern void blink_antenna(void);
extern void blink_auto(uint8_t blinks);
extern void blink_HiLoZ(uint8_t blinks);
extern void blink_scale(uint8_t blinks);
extern void blink_thresh(uint8_t blinks);
extern void show_peak(void);

/* -------------------------------------------------------------------------- */

// function-related, single frame display functions
// THESE HAVE NO DELAYS
extern void show_auto(void);
extern void show_HiLoZ(void);
extern void show_relays(void);
extern void show_scale(void);
extern void show_thresh(void);

/* -------------------------------------------------------------------------- */

extern void show_current_power_and_SWR(void);
extern int shell_show_bargraphs(int argc, char** argv);

#endif