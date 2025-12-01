#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

#define HEADER_FRAME 0
#define END_FRAME 0

typedef struct {
    uint8_t upper;
    uint8_t lower;
    int16_t frame_delay;
} animation_s;

/* -------------------------------------------------------------------------- */
/*  Animations

HEADER_FRAME is a metadata frame that lets you declare that you WILL NOT
use one of the two bargraphs. This prevents the animation system from
overwriting anything that might already be on the display.

const animation_s example1[] = {
    {true, false, HEADER_FRAME}, // <-- HEADER_FRAME
    {0xff, 0x00, 250},
    {0x00, 0x00, 250},
    {0x00, 0x00, END_FRAME},
};

END_FRAME is an animation null terminator.
The content of the end frame is left on the display.

const animation_s example2[] = {
    {0xff, 0xff, 100},
    {0x00, 0x00, 100},
    {0x00, 0x00, END_FRAME}, // <-- END_FRAME
};

You can also terminate an animation with a negative delay.
This is equivalent to using a positive delay plus an END_FRAME.

const animation_s example3[] = {
    {0x81, 0x81, 100},
    {0x42, 0x42, 100},
    {0x24, 0x24, 100},
    {0x18, 0x18, -100}, // <-- negative delay
};

const animation_s example4[] = {
    {0x81, 0x81, 100},
    {0x42, 0x42, 100},
    {0x24, 0x24, 100},
    {0x18, 0x18, 100},
    {0x00, 0x00, END_FRAME}, // <-- END_FRAME
};
*/

//  upper, lower, delay
const animation_s right_crawl[] = {
    {0x01, 0x01, 100}, //
    {0x02, 0x02, 100}, //
    {0x04, 0x04, 75},  //
    {0x08, 0x08, 75},  //
    {0x10, 0x10, 75},  //
    {0x20, 0x20, 50},  //
    {0x40, 0x40, 50},  //
    {0x80, 0x80, -50}, //
};

const animation_s left_crawl[] = {
    {0x80, 0x80, 100}, //
    {0x40, 0x40, 100}, //
    {0x20, 0x20, 75},  //
    {0x10, 0x10, 75},  //
    {0x08, 0x08, 75},  //
    {0x04, 0x04, 50},  //
    {0x02, 0x02, 50},  //
    {0x01, 0x01, -50}, //
};

const animation_s center_crawl[] = {
    {0x81, 0x81, 100},  //
    {0x42, 0x42, 100},  //
    {0x24, 0x24, 100},  //
    {0x18, 0x18, -100}, //
};

const animation_s arrow_down[] = {
    {0x18, 0x00, 100},  //
    {0x24, 0x18, 100},  //
    {0x00, 0x00, 100},  //
    {0x24, 0x18, 100},  //
    {0x00, 0x00, 100},  //
    {0x24, 0x18, -100}, //
};

const animation_s arrow_up[] = {
    {0x00, 0x18, 100},  //
    {0x18, 0x24, 100},  //
    {0x00, 0x00, 100},  //
    {0x18, 0x24, 100},  //
    {0x00, 0x00, 100},  //
    {0x18, 0x24, -100}, //
};

const animation_s blink_both_bars[] = {
    {0xff, 0xff, 100},  //
    {0x00, 0x00, -100}, //
};

const animation_s blink_top_bar[] = {
    {true, false, HEADER_FRAME}, //
    {0xff, 0x00, 250},           //
    {0x00, 0x00, -250},          //
};

const animation_s blink_bottom_bar[] = {
    {false, true, HEADER_FRAME}, //
    {0x00, 0xff, 250},           //
    {0x00, 0x00, -250},          //
};

const animation_s left_wave[] = {
    {0x30, 0x30, 200},  //
    {0xc0, 0xc0, 100},  //
    {0x00, 0x00, 100},  //
    {0xc0, 0xc0, -100}, //
};

const animation_s left_blink[] = {
    {0xc0, 0xc0, 100},  //
    {0x00, 0x00, 100},  //
    {0xc0, 0xc0, -100}, //
};

const animation_s right_wave[] = {
    {0x0c, 0x0c, 200},  //
    {0x03, 0x03, 100},  //
    {0x00, 0x00, 100},  //
    {0x03, 0x03, -100}, //
};

const animation_s right_blink[] = {
    {0x03, 0x03, 100},  //
    {0x00, 0x00, 100},  //
    {0x03, 0x03, -100}, //
};

const animation_s toggle_outer_leds[] = {
    {0xa5, 0x42, 100},  //
    {0x42, 0xa5, -100}, //
};

const animation_s toggle_inner_leds[] = {
    {0x14, 0x28, 100},  //
    {0x28, 0x14, -100}, //
};

/* -------------------------------------------------------------------------- */
// modes

const animation_s auto_mode[][2] = { //
    {
        {0x81, 0x81, 100},  //
        {0x00, 0x00, -100}, //
    },                      //
    {
        {0x18, 0x18, 100},  //
        {0x00, 0x00, -100}, //
    }};

const animation_s hiz_wave[] = {
    {0xc0, 0xc0, 100},  //
    {0x00, 0x00, -100}, //
};

const animation_s loz_wave[] = {
    {0x03, 0x03, 100},  //
    {0x00, 0x00, -100}, //
};

const animation_s high_scale[] = {
    {0x02, 0x00, 100},  //
    {0x00, 0x00, -100}, //
};

const animation_s low_scale[] = {
    {0x80, 0x00, 100},  //
    {0x00, 0x00, -100}, //
};

const animation_s peak_on[] = {
    {0x1f, 0x1f, 200},  //
    {0x17, 0x17, 100},  //
    {0x13, 0x13, 100},  //
    {0x09, 0x09, 100},  //
    {0x08, 0x08, 300},  //
    {0x04, 0x04, -400}, //
};

const animation_s peak_off[] = {
    {0x1f, 0x1f, 100},  //
    {0x0f, 0x0f, 100},  //
    {0x07, 0x07, 100},  //
    {0x03, 0x03, 100},  //
    {0x01, 0x01, -100}, //
};

const animation_s swrThreshold[][2] = {
    {
        {0x00, 0x08, 100},  //
        {0x00, 0x00, -100}, //
    },
    {
        {0x00, 0x10, 100},  //
        {0x00, 0x00, -100}, //
    },
    {
        {0x00, 0x20, 100},  //
        {0x00, 0x00, -100}, //
    },
    {
        {0x00, 0x40, 100},  //
        {0x00, 0x00, -100}, //
    },
};

/* -------------------------------------------------------------------------- */
// misc

#if DEVELOPMENT

const animation_s clockwise_idle[] = {
    {0x00, 0x01, 100},  //
    {0x00, 0x02, 100},  //
    {0x00, 0x04, 100},  //
    {0x00, 0x08, 100},  //
    {0x00, 0x10, 100},  //
    {0x00, 0x20, 100},  //
    {0x00, 0x40, 100},  //
    {0x00, 0x80, 100},  //
    {0x80, 0x00, 100},  //
    {0x40, 0x00, 100},  //
    {0x20, 0x00, 100},  //
    {0x10, 0x00, 100},  //
    {0x08, 0x00, 100},  //
    {0x04, 0x00, 100},  //
    {0x02, 0x00, 100},  //
    {0x01, 0x00, -100}, //
};
#endif

#endif