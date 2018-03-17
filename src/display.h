#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* ************************************************************************** */

enum blink_speeds {
    FAST = 100,
    MEDIUM = 300,
    SLOW = 500,
    VERY_SLOW = 1000,
};

/* ************************************************************************** */

extern void display_init(void);

/* -------------------------------------------------------------------------- */

// generic display functions
extern void display_clear(void);
extern void display_raw_frame(uint16_t frame);
extern void display_single_frame(const animation_s *animation, uint8_t frame_number);

extern void play_animation(const animation_s *animation);
extern void repeat_animation(const animation_s *animation, uint8_t repeats);
extern void play_interruptable_animation(const animation_s *animation);

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

// function-related, single frame display functions
// THESE HAVE NO DELAYS
extern void update_antenna_led(void);
extern void update_bypass_led(void);
extern void update_power_led(void);

extern void show_auto(void);
extern void show_HiLoZ(void);
extern void show_relays(void);

extern void show_scale(void);
extern void show_thresh(void);

/* -------------------------------------------------------------------------- */

#endif