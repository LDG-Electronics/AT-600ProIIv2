#ifndef _ANIMATIONS_H_
#define _ANIMATIONS_H_

/* ************************************************************************** */
typedef struct {
    uint16_t image;
    uint16_t frame_delay;
} animation_s;

const animation_s startup_frames[9] = {
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

const animation_s arrow_up_frames[7] = {
    {0x1800,100},
    {0x2418,100},
    {0x0000,100},
    {0x2418,100},
    {0x0000,100},
    {0x2418,100},
    {0x0000,NULL},
};

const animation_s arrow_down_frames[7] = {
    {0x0018,100},
    {0x1824,100},
    {0x0000,100},
    {0x1824,100},
    {0x0000,100},
    {0x1824,100},
    {0x0000,NULL},
};

#endif