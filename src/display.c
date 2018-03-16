#include "includes.h"

/* ************************************************************************** */

const uint8_t swrThreshDisplay[4] = { 0x08,0x10,0x20,0x40 };

const uint16_t ledBarTable[9] = { 0x00,0x01,0x03,0x07,0x0f,0x1f,0x3f,0x7f,0xff, };
const uint16_t ledSingleTable[9] = { 0x00,0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80, };

/* ************************************************************************** */

void FP_update(uint16_t d)
{
    uint8_t i;
    FP_STROBE_PIN = 1;
    FP_CLOCK_PIN = 0;
    for (i = 0; i < 16; i++)
    {
        if (d & (1 << (15 - i))) {
            FP_DATA_PIN = 1;
        } else {
            FP_DATA_PIN = 0;
        }
        delay_10us(1);
        FP_CLOCK_PIN = 1;
        delay_10us(1);
        FP_CLOCK_PIN = 0;
        delay_10us(1);
    }
    FP_STROBE_PIN = 0;
    delay_10us(1);
    FP_STROBE_PIN = 1;
    delay_10us(1);
}

/* ************************************************************************** */

void display_init(void)
{
    // Clear Front Panel bitbang SPI pins
    FP_CLOCK_PIN = 0;
    FP_DATA_PIN = 0;
    FP_STROBE_PIN = 0;

    POWER_LED_PIN = 0;
    ANT_LED_PIN = 0;
    BYPASS_LED_PIN = 0;
}

/* -------------------------------------------------------------------------- */

void display_clear(void)
{
    FP_update(0x0000);
}

void display_raw_frame(uint16_t frame)
{
    FP_update(frame);
}

void display_single_frame(const animation_s *animation, uint8_t frame_number)
{
    FP_update(animation[frame_number].image);
}

void play_animation(const animation_s *animation)
{
    uint8_t i = 0;

    while(1)
    {
        FP_update(animation[i].image);
        if (animation[i].frame_delay == NULL) break;
        delay_ms(animation[i].frame_delay);

        i++;
    }
}

void repeat_animation(const animation_s *animation, uint8_t repeats)
{
    uint8_t i;

    for (i = 0; i < repeats; i++)
    {
        play_animation(animation);
    }
}

void play_interruptable_animation(const animation_s *animation)
{
    uint8_t i = 0;
    uint16_t j = 0;

    while(1)
    {
        FP_update(animation[i].image);
        if (animation[i].frame_delay == NULL) break;
        
        for(j = animation[i].frame_delay; j != 0; j--) {
            if (get_buttons2() != 0) return;
            delay_ms(1);
        }

        i++;
    }
}

/* -------------------------------------------------------------------------- */

// function-related display functions

void show_peak(void)
{
    if (system_flags.PeakOn == 0) {
        play_animation(&peak_off);
    } else {
        play_animation(&peak_on);
    }
}

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS
void blink_antenna(void)
{
    if (system_flags.Antenna == 1) {
        play_animation(&right_wave);
    } else {
        play_animation(&left_wave);
    }
}

void blink_auto(uint8_t blinks)
{
    if (system_flags.AutoMode == 0) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

void blink_HiLoZ(uint8_t blinks)
{
    if (currentRelays[currentAntenna].z == 1) {
        repeat_animation(&auto_off, blinks);
    } else {
        repeat_animation(&auto_on, blinks);
    }
}

void blink_scale(uint8_t blinks)
{
    if (system_flags.Scale100W == 0) {
        repeat_animation(&high_scale, blinks);
    } else {
        repeat_animation(&low_scale, blinks);
    }
}

void blink_thresh(uint8_t blinks)
{
    uint8_t i;
    uint16_t out;      
    out = (swrThreshDisplay[swrThreshIndex] << 8);
    
    for (i = 0; i < blinks; i++)
    {
        FP_update(out);
        delay_ms(100);
        FP_update(0x0000);
        delay_ms(100);
    }
}

// function-related, single frame display functions
// THESE HAVE NO DELAYS
void update_antenna_led(void)
{
    ANT_LED_PIN = ~currentAntenna;
}

void update_bypass_led(void)
{
    BYPASS_LED_PIN = system_flags.inBypass;
}

void update_power_led(void)
{
    //TODO: implement power-on status flag 
    // POWER_LED_PIN = system_flags.inBypass;
}

void show_auto(void)
{
    if (system_flags.AutoMode == 0) {
        FP_update(0x8181);
    } else {
        FP_update(0x1818);
    }
}

void show_HiLoZ(void)
{
    if (currentRelays[currentAntenna].z == 1) {
        FP_update(0xc0c0);
    } else {
        FP_update(0x0303);
    }
}

void show_relays(void)
{
    uint16_t out;

    out = ((currentRelays[currentAntenna].inds & 0x7f) | ((currentRelays[currentAntenna].z & 0x01) << 7));
    out |= (uint16_t)currentRelays[currentAntenna].caps << 8;

    FP_update(out);
}

void show_scale(void)
{
    if (system_flags.Scale100W == 0) {
        FP_update(0x0002);
    } else {
        FP_update(0x0040);
    }
}

void show_thresh(void)
{
    FP_update(swrThreshDisplay[swrThreshIndex] << 8);
}


