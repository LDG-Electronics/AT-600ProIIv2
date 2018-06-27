#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

/* ************************************************************************** */
typedef struct {
    uint16_t image;
    uint16_t frame_delay;
} animation_s;

/* -------------------------------------------------------------------------- */
// shapes

const animation_s right_crawl[] = {
    {0x0101,100},
    {0x0202,100},
    {0x0404,75},
    {0x0808,75},
    {0x1010,75},
    {0x2020,50},
    {0x4040,50},
    {0x8080,50},
    {0x0000,NULL},
};

const animation_s left_crawl[] = {
    {0x8080,100},
    {0x4040,100},
    {0x2020,75},
    {0x1010,75},
    {0x0808,75},
    {0x0404,50},
    {0x0202,50},
    {0x0101,50},
    {0x0000,NULL},
};

const animation_s center_crawl[] = {
    {0x8181,100},
    {0x4242,100},
    {0x2424,100},
    {0x1818,100},
    {0x0000,NULL},
};

const animation_s arrow_up[] = {
    {0x1800,100},
    {0x2418,100},
    {0x0000,100},
    {0x2418,100},
    {0x0000,100},
    {0x2418,100},
    {0x0000,NULL},
};

const animation_s arrow_down[] = {
    {0x0018,100},
    {0x1824,100},
    {0x0000,100},
    {0x1824,100},
    {0x0000,100},
    {0x1824,100},
    {0x0000,NULL},
};

const animation_s railroad_crossing[] = {
    {0x42a5,200},
    {0xa542,200},
    {0x0000,NULL},
};

const animation_s blink_both_bars[] = {
    {0xffff,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s blink_top_bar[] = {
    {0xff00,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s blink_top_bar_3[] = {
    {0xff00,100},
    {0x0000,100},
    {0xff00,100},
    {0x0000,100},
    {0xff00,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s blink_bottom_bar[] = {
    {0x00ff,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s blink_bottom_bar_3[] = {
    {0x00ff,100},
    {0x0000,100},
    {0x00ff,100},
    {0x0000,100},
    {0x00ff,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s left_wave[] = {
    {0x3030,100},
    {0xc0c0,100},
    {0x0000,NULL},
};

const animation_s right_wave[] = {
    {0x0c0c,100},
    {0x0303,100},
    {0x0000,NULL},
};

/* -------------------------------------------------------------------------- */
// modes

const animation_s auto_on[] = {
    {0x1818,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s auto_off[] = {
    {0x8181,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s hiz_wave[] = {
    {0xc0c0,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s loz_wave[] = {
    {0x0303,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s high_scale[] = {
    {0x0008,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s low_scale[] = {
    {0x0080,100},
    {0x0000,100},
    {0x0000,NULL},
};

const animation_s peak_on[] = {
    {0x1f1f,200},
    {0x1717,100},
    {0x1313,100},
    {0x0909,100},
    {0x0808,300},
    {0x0404,400},
    {0x0000,NULL},
};

const animation_s peak_off[] = {
    {0x1f1f,100},
    {0x0f0f,100},
    {0x0707,100},
    {0x0303,100},
    {0x0101,100},
    {0x0000,NULL},
};

/* -------------------------------------------------------------------------- */
// misc

const animation_s clockwise_idle[] = {
    {0x0001,100},
    {0x0002,100},
    {0x0004,100},
    {0x0008,100},
    {0x0010,100},
    {0x0020,100},
    {0x0040,100},
    {0x0080,100},
    {0x8000,100},
    {0x4000,100},
    {0x2000,100},
    {0x1000,100},
    {0x0800,100},
    {0x0400,100},
    {0x0200,100},
    {0x0100,100},
    {0x0000,NULL},
};

const animation_s dual_clockwise_idle[] = {
    {0x0011,100},
    {0x0022,100},
    {0x0044,100},
    {0x0088,100},
    {0x8800,100},
    {0x4400,100},
    {0x2200,100},
    {0x1100,100},    
    {0x0000,NULL},
};

const animation_s counter_clockwise_idle[] = {
    {0x0100,100},
    {0x0200,100},
    {0x0400,100},
    {0x0800,100},
    {0x1000,100},
    {0x2000,100},
    {0x4000,100},
    {0x8000,100},
    {0x0080,100},
    {0x0040,100},
    {0x0020,100},
    {0x0010,100},
    {0x0008,100},
    {0x0004,100},
    {0x0002,100},
    {0x0001,100},
    {0x0000,NULL},
};

const animation_s dual_counter_clockwise_idle[] = {
    {0x1100,100},
    {0x2200,100},
    {0x4400,100},
    {0x8800,100},
    {0x0088,100},
    {0x0044,100},
    {0x0022,100},
    {0x0011,100},  
    {0x0000,NULL},
};

#endif