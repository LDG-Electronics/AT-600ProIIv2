#include "includes.h"

/* ************************************************************************** */
#define SUBMENU_TIMEOUT 2200

void threshold_submenu(void)
{
    uint16_t modeCount = SUBMENU_TIMEOUT;
    
    blink_thresh(3);
    show_thresh();
    
    while (1)
    {
        if (btn_is_pressed(LDN))
        {
            modeCount = 0;
            SWR_threshold_increment();
            
            show_thresh();
            blink_thresh(2);
            show_thresh();
        }        
        
        if (btn_is_pressed(FUNC)) break;
        modeCount--;
        if (modeCount == 0) break;
        delay_ms(1);
    }
}

void function_submenu(void)
{
    uint16_t modeCount = SUBMENU_TIMEOUT; 

    play_interruptable_animation(&arrow_up);

    while (1)
    {
        if (btn_is_pressed(CUP)) {
            toggle_peak();
            show_peak();
            return;
        }
        if (btn_is_pressed(LUP)) {
            toggle_scale();
            blink_scale(4);
            return;
        }
        if (btn_is_pressed(CDN)) {
            toggle_auto();
            blink_auto(4);
            return;
        }
        if (btn_is_pressed(LDN)) {
            threshold_submenu();
            blink_thresh(4);
            return;
        }
        if (btn_is_pressed(TUNE)) {
            manual_store();
            return;
        }
        if (btn_is_pressed(POWER)) {
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }

        if (btn_is_pressed(FUNC)) break;
        modeCount--;
        if (modeCount == 0) break;
        delay_ms(1);
    }
    play_animation(&arrow_down);
}

/* -------------------------------------------------------------------------- */

void cup_hold(void)
{
    while(btn_is_down(CUP))
    {
        capacitor_increment();
        if(btn_is_up(CUP)) break;

        delay_ms(1);
    }
}

void cdn_hold(void)
{
    while(btn_is_down(CDN))
    {
        capacitor_decrement();
        if(btn_is_up(CDN)) break;

        delay_ms(1);
    }
}

void lup_hold(void)
{
    while(btn_is_down(LUP))
    {
        inductor_increment();
        if(btn_is_up(LUP)) break;

        delay_ms(1);
    }
}

void ldn_hold(void)
{
    while(btn_is_down(LDN))
    {
        inductor_decrement();
        if(btn_is_up(LDN)) break;

        delay_ms(1);
    }
}

void tune_hold(void)
{
    uint16_t buttonCount = 0;

    while(btn_is_down(TUNE))
    {
        if (buttonCount < UINT16_MAX) buttonCount++;

        if (buttonCount < BTN_PRESS_DEBOUNCE) {
            // button was not held long enough, do nothing
        } else if (buttonCount < BTN_PRESS_SHORT) {
            display_single_frame(&center_crawl, 0);
            // display_raw_frame(center_crawl[0].image);
        } else if (buttonCount < BTN_PRESS_MEDIUM) {
            display_single_frame(&center_crawl, 2);
            // display_raw_frame(center_crawl[2].image);
        } else if (buttonCount < BTN_PRESS_LONG) {
            display_single_frame(&center_crawl, 3);
            // display_raw_frame(center_crawl[3].image);
        } else if (buttonCount >= BTN_PRESS_LONG) {
            display_clear();
        }

        delay_ms(1);
    }

    display_clear();

    if (buttonCount < BTN_PRESS_DEBOUNCE) {
        // button was not held long enough, do nothing
    } else if (buttonCount < BTN_PRESS_SHORT) {
        toggle_bypass();
    } else if (buttonCount < BTN_PRESS_MEDIUM) {
        // request_memory_tune();
        request_full_tune();
    } else if (buttonCount < BTN_PRESS_LONG) {
        request_full_tune();
    } else if (buttonCount >= BTN_PRESS_LONG) {
        // button was held for too long, do nothing
    }
}

void func_hold(void)
{
    uint8_t FuncHoldProcessed = 0;

    while(btn_is_down(FUNC))
    {
        if (btn_is_pressed(CUP)) {
            FuncHoldProcessed = 1;
            show_peak();
            // empty
        }
        if (btn_is_pressed(LUP)) {
            FuncHoldProcessed = 1;
            blink_HiLoZ(2);
            show_HiLoZ();
        }
        if (btn_is_pressed(CDN)) {
            FuncHoldProcessed = 1;
            blink_auto(4);
            show_auto();
        }
        if (btn_is_pressed(LDN)) {
            FuncHoldProcessed = 1;
            blink_thresh(4);
            show_thresh();
        }
    }

    if (FuncHoldProcessed == 0) function_submenu();
}

void ant_hold(void)
{
    toggle_antenna();
    blink_antenna();
    update_antenna_led();

    //! This function is configured for the POWER button instead of ANT
    // The ANT button is disabled on the dev unit
    while(btn_is_down(POWER))
    {
        if (btn_is_released(POWER))
        {
            break;
        }
    }
}

void power_hold(void)
{
    uint16_t buttonCount = 0;

    print_str_ln("power_hold");

    while(btn_is_down(POWER))
    {
        if (buttonCount < UINT16_MAX) buttonCount++;

        if (buttonCount == 1000) {
            print_str_ln("going to sleep");
        }

        if (btn_is_released(POWER))
        {
            break;
        }
    }
}