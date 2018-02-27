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

    POWER_LED = 0;
    ANT_LED = 0;
    BYPASS_LED = 0;

    play_animation(&startup_frames);
}

/* -------------------------------------------------------------------------- */



void play_animation(animation_s *animation)
{
    uint8_t i = 0;

    while(1)
    {
        FP_update(animation[i].image);
        if (animation[i].frame_delay == 0) break;
        delay_ms(animation[i].frame_delay);

        i++;
    }
}

/* -------------------------------------------------------------------------- */

// generic display functions
void show_frame(uint8_t command, uint16_t argument)
{
    switch (command)
    {
    case CLEAR:
        FP_update(0x0000);
        break;
        
    case RAW_LEDS:
        FP_update(argument);
        break;
        
    case PWR_ALL:
        if (argument == 1)
            FP_update(0x00ff);
        else
            FP_update(0x0000);
        break;
        
    case PWR_BAR:
        if (argument > 10)
            FP_update(ledBarTable[argument]);
        else
            FP_update(0x0000);
        break;
        
    case PWR_SINGLE:
        if (argument > 10)
            FP_update(ledSingleTable[argument]);
        else
            FP_update(0x0000);
        break;
        
    case SWR_ALL:
        if (argument == 1)
            FP_update(0xff00);
        else
            FP_update(0x0000);
        break;
        
    case SWR_BAR:
        if (argument > 10)
            FP_update(ledBarTable[argument] << 8);
        else
            FP_update(0x0000);
        break;
        
    case SWR_SINGLE:
        if (argument > 10)
            FP_update(ledSingleTable[argument] << 8);
        else
            FP_update(0x0000);
        break;
        
    case BOTH_ALL:
        if (argument == 1)
            FP_update(0xffff);
        else
            FP_update(0x0000);
        break;
        
    case BOTH_BAR:
        if (argument > 10)
            FP_update((ledBarTable[argument] << 8) && ledBarTable[argument]);
        else
            FP_update(0x0000);
        break;
        
    case BOTH_SINGLE:
        if (argument > 10)
            FP_update((ledSingleTable[argument] << 8) && ledSingleTable[argument]);
        else
            FP_update(0x0000);
        break;
        
    case ARROW_UP_FRAMES:
        if (argument == 1)
            FP_update(0x1800);
        else if (argument == 2)
            FP_update(0x2418);
        else
            FP_update(0x0000);
        break;
        
    case ARROW_DOWN_FRAMES:
        if (argument == 1)
            FP_update(0x0018);
        else if (argument == 2)
            FP_update(0x1824);
        else
            FP_update(0x0000);
        break;

    case TUNE_FRAMES:
        if (argument == 1)
            FP_update(0x8181);
        else if (argument == 2)
            FP_update(0x4242);
        else if (argument == 3)
            FP_update(0x2424);
        else if (argument == 4)
            FP_update(0x1818);
        else
            FP_update(0x0000);
        break;
        
    case POWER_ERROR:
        if (argument == 1)
            FP_update(0x42a5);
        else if (argument == 2)
            FP_update(0xa542);
        else
            FP_update(0x0000);
        break;
        
    case SWR_ERROR:
        if (argument == 1)
            FP_update(0x2814);
        else if (argument == 2)
            FP_update(0x1428);
        else
            FP_update(0x0000);
        break;
        
    case WAVE_LEFT:
        if (argument == 1)
            FP_update(0x3030);
        else if (argument == 2)
            FP_update(0xc0c0);
        else
            FP_update(0x0000);
        break;
        
    case WAVE_RIGHT:
        if (argument == 1)
            FP_update(0x0c0c);
        else if (argument == 2)
            FP_update(0x0303);
        else
            FP_update(0x0000);
        break;
        
    case FUNC_HOLD:
        if (argument == 1)
            FP_update(0x1800);
        else if (argument == 2)
            FP_update(0x0018);
        else
            FP_update(0x0000);
        break;
    }
}

void show_animation(uint8_t command)
{
    uint16_t i;
    
    switch (command)
    {
    case STARTUP:
        for (i = 0; i < 10; i++)
        {
            show_frame(BOTH_SINGLE, i);
            delay_ms(75);
        }   
        show_frame(CLEAR, 0);
        break;
        
    case PWR_ALL_1BLINK:
        show_frame(PWR_ALL, 1);
        delay_ms(LED_BLINK_SLOW);
        show_frame(PWR_ALL, 0);
        delay_ms(LED_BLINK_SLOW);
        break;
        
    case PWR_ALL_2BLINK:        
        show_frame(PWR_ALL, 1);
        delay_ms(LED_BLINK_SLOW);
        show_frame(PWR_ALL, 0);
        delay_ms(LED_BLINK_SLOW);
        show_frame(PWR_ALL, 1);
        delay_ms(LED_BLINK_SLOW);
        show_frame(PWR_ALL, 0);
        delay_ms(LED_BLINK_SLOW);
        break;
        
    case SWR_ALL_1BLINK:
        show_frame(SWR_ALL, 1);
        delay_ms(LED_BLINK_SLOW);
        show_frame(SWR_ALL, 0);
        delay_ms(LED_BLINK_SLOW);
        break;
        
    case SWR_ALL_2BLINK:
        show_frame(SWR_ALL, 1);
        delay_ms(LED_BLINK_MED);
        show_frame(SWR_ALL, 0);
        delay_ms(LED_BLINK_MED);
        show_frame(SWR_ALL, 1);
        delay_ms(LED_BLINK_MED);
        show_frame(SWR_ALL, 0);
        delay_ms(LED_BLINK_MED);
        break;
        
    case BOTH_ALL_1BLINK:
        show_frame(BOTH_ALL, 1);
        delay_ms(LED_BLINK_V_SLOW);
        show_frame(BOTH_ALL, 0);
        delay_ms(LED_BLINK_SLOW);
        break;

    case FUNC_TUNE_OKAY:
        for(i = 0; i < 5; i++)
        {
            show_frame(TUNE_FRAMES, i);
            delay_ms(LED_BLINK_MED);
        }
        delay_ms(LED_BLINK_SLOW + 100);
        show_frame(TUNE_FRAMES, 0);
        break;

    case FUNC_TUNE_FAIL:
        show_frame(BOTH_ALL, 0);
        delay_ms(LED_BLINK_MED);
        show_frame(BOTH_ALL, 1);
        delay_ms(LED_BLINK_SLOW + 100);
        show_frame(BOTH_ALL, 0);
        break;

    case TUNE_MOM_ON:
        show_frame(TUNE_FRAMES, 1);
        break;
    case TUNE_MED_ON:
        show_frame(TUNE_FRAMES, 3);
        break;
    case TUNE_FULL_ON:
        show_frame(TUNE_FRAMES, 4);
        break;
        
    case OVER_PWR_ERROR:
        for (i = 0; i < 3; i++)
        {
            show_frame(POWER_ERROR, 1);
            delay_ms(LED_BLINK_SLOW);
            show_frame(POWER_ERROR, 2);
            delay_ms(LED_BLINK_SLOW);
        }
        show_frame(POWER_ERROR, 0);
        break;
        
    case OVER_SWR_PWR:
        for (i = 0; i < 3; i++)
        {
            show_frame(SWR_ERROR, 1);
            delay_ms(LED_BLINK_SLOW);
            show_frame(SWR_ERROR, 2);
            delay_ms(LED_BLINK_SLOW);
        }
        show_frame(SWR_ERROR, 0);
        break;
    }
}

// function-related display functions
void blink_upper_bar(uint8_t blinks)
{
    uint8_t i;

    for (i = 0; i < blinks; i++)
    {
        delay_ms(100);
        FP_update(0xff00);
        delay_ms(100);
        FP_update(0x0000);
    }  
} 

void blink_lower_bar(uint8_t blinks)
{
    uint8_t i;

    for (i = 0; i < blinks; i++)
    {
        delay_ms(100);
        FP_update(0x00ff);
        delay_ms(100);
        FP_update(0x0000);
    }  
} 

void blink_all(uint8_t blinks)
{
    uint8_t i;

    for (i = 0; i < blinks; i++)
    {
        delay_ms(100);
        FP_update(0xffff);
        delay_ms(100);
        FP_update(0x0000);
    }  
} 

void show_peak(void)
{
    if (saved_flags.PeakOn == 0)
    {
        // No peak
        FP_update(0x1f1f);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0f0f);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0707);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0303);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0101);
        delay_ms(LED_BLINK_MED);
        FP_update(0);
        delay_ms(LED_BLINK_MED);

    }
    else
    {
        // show peak
        FP_update(0x1f1f);
        delay_ms(LED_BLINK_SLOW);
        FP_update(0x1717);
        delay_ms(LED_BLINK_MED);
        FP_update(0x1313);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0909);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0808);
        delay_ms(LED_BLINK_SLOW + 100);
        FP_update(0x0404);
        delay_ms(LED_BLINK_SLOW);
        delay_ms(LED_BLINK_SLOW);
        FP_update(0);
        delay_ms(LED_BLINK_MED);
    }
}

void blink_arrow_up(uint8_t blinks)
{
    uint8_t i;
    uint8_t j = 0;
    
    show_frame(ARROW_UP_FRAMES, 0);

    for (i = 0; i < blinks; i++)
    {
        for (j = 0; j < 20; j++)
        {
            delay_ms(5);
            if (get_buttons() != 0)
                goto EarlyFuncButtonPress;
        }
        show_frame(ARROW_UP_FRAMES, 2);
        for (j = 0; j < 20; j++)
        {
            delay_ms(5);
            if (get_buttons() != 0)
                goto EarlyFuncButtonPress;
        }
        show_frame(ARROW_UP_FRAMES, 0);
    }
    
    EarlyFuncButtonPress:
    show_frame(ARROW_UP_FRAMES, 0);
}

void blink_arrow_down(uint8_t blinks)
{
    uint8_t i;

    for (i = 0; i < blinks; i++)
    {
        delay_ms(100);
        show_frame(ARROW_DOWN_FRAMES, 2);
        delay_ms(100);
        show_frame(ARROW_DOWN_FRAMES, 0);
    }  
} 

// function-related, blinky display functions
// THESE HAVE BLOCKING DELAYS
void blink_antenna(void)
{
    if (saved_flags.Antenna == 1)
    {
        ANT_LED = 0;
        FP_update(0x0c0c);
        delay_ms(LED_BLINK_MED);
        FP_update(0x0303);
    }
    else
    {
        ANT_LED = 1;
        FP_update(0x3030);
        delay_ms(LED_BLINK_MED);
        FP_update(0xc0c0);
    }
    delay_ms(LED_BLINK_V_SLOW);
    FP_update(0x0000);
}

void blink_auto(uint8_t blinks)
{
    uint8_t i;
    for (i = 0; i < blinks; i++)
    {
        if (saved_flags.AutoMode == 0) {
            FP_update(0x8181);
        } else {
            FP_update(0x1818);
        }
        delay_ms(LED_BLINK_FAST);
        FP_update(0x0000);
        delay_ms(LED_BLINK_FAST);
    }
}

void blink_HiLoZ(uint8_t blinks)
{
    uint8_t i;
    uint16_t out;
    if (currentRelays.z == 1){out = 0xc0c0;}
    else{out = 0x0303;}
    
    for (i = 0; i < blinks; i++)
    {
        FP_update(out);
        delay_ms(LED_BLINK_FAST);
        FP_update(0x0000);
        delay_ms(LED_BLINK_FAST);
    }
}

void blink_scale(uint8_t blinks)
{
    uint8_t i;
    for (i = 0; i < blinks; i++)
    {

        if (saved_flags.Scale100W == 0) {
            FP_update(0x0002);
            delay_ms(LED_BLINK_FAST);
        } else {
            FP_update(0x0040);
            delay_ms(LED_BLINK_FAST);
        }
        FP_update(0);
        delay_ms(LED_BLINK_FAST);
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
        delay_ms(LED_BLINK_FAST);
        FP_update(0x0000);
        delay_ms(LED_BLINK_FAST);
    }
}

// function-related, single frame display functions
// THESE HAVE NO DELAYS
void show_antenna_led(void)
{
    if (saved_flags.Antenna == 1) {
        ANT_LED = 0;
    } else {
        ANT_LED = 1;
    }
    return;
}

void show_auto(void)
{
    if (saved_flags.AutoMode == 0) {
        FP_update(0x8181);
    } else {
        FP_update(0x1818);
    }
    return;
}

void show_HiLoZ(void)
{
    if (currentRelays.z == 1) {
        FP_update(0xc0c0);
    } else {
        FP_update(0x0303);
    }
    return;
}

void show_bypass(void)
{
    if (saved_flags.inBypass == 0) {
        BYPASS_LED = 0;
    } else {
        BYPASS_LED = 1;
    }
    return;
}

void show_relays(void)
{
    uint16_t out;
    out = ((currentRelays.inds & 0x7f) | ((currentRelays.z & 0x01) << 7));
    out |= (uint16_t)currentRelays.caps << 8;
    FP_update(out);
    return;
}

void show_scale(void)
{
    if (saved_flags.Scale100W == 0) {
        FP_update(0x0002);
    } else {
        FP_update(0x0040);
    }
    return;
}

void show_thresh(void)
{
    show_frame(RAW_LEDS, (swrThreshDisplay[swrThreshIndex] << 8));
    return;
}


