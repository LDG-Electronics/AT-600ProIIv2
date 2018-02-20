#ifndef _DISPLAY_H_
#define _DISPLAY_H_

/* ************************************************************************** */

// frame command codes
#define CLEAR               0
#define RAW_LEDS            1
#define FUNC_HOLD           9

#define PWR_ALL             10
#define PWR_BAR             11
#define PWR_SINGLE          12
#define SWR_ALL             13
#define SWR_BAR             14
#define SWR_SINGLE          15
#define BOTH_ALL            16
#define BOTH_BAR            17
#define BOTH_SINGLE         18

#define ARROW_UP_FRAMES     20
#define ARROW_DOWN_FRAMES   21
#define TUNE_FRAMES         22
#define POWER_ERROR         23
#define SWR_ERROR           24
#define WAVE_LEFT           25
#define WAVE_RIGHT          26

// animation command codes
#define STARTUP             30

#define PWR_ALL_1BLINK      41
#define PWR_ALL_2BLINK      42
#define SWR_ALL_1BLINK      43
#define SWR_ALL_2BLINK      44
#define BOTH_ALL_1BLINK     45
#define BOTH_ALL_2BLINK     46

#define FUNC_LOW_PWR        52
#define FUNC_HIGH_PWR       53
#define FUNC_TUNE_OKAY      54
#define FUNC_TUNE_FAIL      55

#define TUNE_MOM_ON         80
#define TUNE_MED_ON         81
#define TUNE_FULL_ON        82
#define OVER_PWR_ERROR      90
#define OVER_SWR_PWR        91

// Display timing constants
#define LED_BLINK_V_FAST    75
#define LED_BLINK_FAST      100
#define LED_BLINK_MED       150
#define LED_BLINK_SLOW      200
#define LED_BLINK_V_SLOW    250

enum blink_speeds {
    FAST = 100,
    MEDIUM = 300,
    SLOW = 500,
    VERY_SLOW = 1000,
};

/* ************************************************************************** */

extern void display_init(void);

/* -------------------------------------------------------------------------- */

extern void show_startup(void);

/* -------------------------------------------------------------------------- */

// generic display functions
extern void show_frame(uint8_t command, uint16_t arg);
extern void show_animation(uint8_t command);

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS

extern void blink_arrow_up(uint8_t blinks);
extern void blink_arrow_down(uint8_t blinks);

extern void blink_antenna(void);
extern void blink_auto(uint8_t blinks);
extern void blink_HiLoZ(uint8_t blinks);

extern void blink_scale(uint8_t blinks);
extern void blink_thresh(uint8_t blinks);

// function-related, single frame display functions
// THESE HAVE NO DELAYS
extern void show_antenna_led(void);
extern void show_auto(void);
extern void show_HiLoZ(void);
extern void show_bypass(void);
extern void show_relays(void);

extern void show_scale(void);
extern void show_thresh(void);





/* -------------------------------------------------------------------------- */

#endif