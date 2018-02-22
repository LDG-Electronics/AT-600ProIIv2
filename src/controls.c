#include "includes.h"

/* ************************************************************************** */

void toggle_bypass(void)
{
    relays_s undoRelays;
    undoRelays.all = currentRelays.all;

    if (saved_flags.inBypass == 1) {
        currentRelays.all = preBypassRelays.all;

        if (put_relays(&currentRelays) == -1) {
            currentRelays.all = undoRelays.all;
        } else {
            show_relays();
            delay_ms(250);
        }
    } else {
        preBypassRelays.all = currentRelays.all;

        if (put_relays(&bypassRelays) == -1) {
            currentRelays.all = undoRelays.all;
        } else {
            blink_all(3);
        }
    }
}

void toggle_auto(void)
{
    saved_flags.AutoMode = !saved_flags.AutoMode;
}

void toggle_hiloz(void)
{
    uint8_t undo = currentRelays.z;

    currentRelays.z = !currentRelays.z;
    
    if (put_relays(&currentRelays) == -1)
    {
        currentRelays.z = undo;
    }
}

void manual_store(void)
{
    // uint16_t memAddr = 0;
    
    // memAddr = period_to_addr(currentRF.period);
    
    // // If address is within valid range, save it
    // if ((memAddr != 0) && (memAddr < 2046)) {
    //     memAddr = (memAddr << 1) + MEM_BASE_ADDR;
    //     memory_store(memAddr);
    //     // display_animation(FUNC_TUNE_OKAY);
    // } else {
    //     blink_all(2);
    // }  
}

/* -------------------------------------------------------------------------- */

void short_tune_release(void)
{
    toggle_bypass();
}

void medium_tune_release(void)
{
    // poll_for_fwd_pwr(500);
    
    memory_tune();
    
    if (tuning_flags.noMemory == 1)
    {
        full_tune();
    }
    tuning_followup_animation();
}

void long_tune_release(void)
{
    // poll_for_fwd_pwr(500);
    
    full_tune();

    tuning_followup_animation();
}

/* -------------------------------------------------------------------------- */

void mode_thresh(void)
{
    uint16_t modeCount = 0;
    
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
        
        if (btn_is_pressed(FUNC) || modeCount == 2000)
        {
            return;
        }
        modeCount++;
        delay_ms(1);
    }
}

#define FUNC_THRESH 2200

void mode_func(void)
{
    uint8_t buttons = 0;
    uint16_t funcCounter = FUNC_THRESH; 

    blink_arrow_up(3);
    
    while (1)
    {
        if (btn_is_pressed(CUP)) {
            // no function on this button
            return;
        }
        if (btn_is_pressed(LUP)) {
            toggle_hiloz();
            blink_HiLoZ(4);
            return;
        }
        if (btn_is_pressed(CDN)) {
            toggle_auto();
            blink_auto(4);
            return;
        }
        if (btn_is_pressed(LDN)) {
            mode_thresh();
            blink_thresh(4);
            return;
        }
        if (btn_is_pressed(TUNE)) {
            manual_store();
            return;
        }

        if (btn_is_pressed(FUNC)) break;
        funcCounter--;
        if (funcCounter == 0) break;
        delay_ms(1);
    }
    blink_arrow_down(3);
}

/* -------------------------------------------------------------------------- */

void cup_hold(void)
{
    while(1)
    {
        capacitor_increment();
        if(btn_is_up(CUP)) break;

        delay_ms(1);
    }
}

void cdn_hold(void)
{
    while(1)
    {
        capacitor_decrement();
        if(btn_is_up(CDN)) break;

        delay_ms(1);
    }
}

void lup_hold(void)
{
    while(1)
    {
        inductor_increment();
        if(btn_is_up(LUP)) break;

        delay_ms(1);
    }
}

void ldn_hold(void)
{
    while(1)
    {
        inductor_decrement();
        if(btn_is_up(LDN)) break;

        delay_ms(1);
    }
}

void tune_hold(void)
{
    uint16_t buttonCount = 0;

    while(1)
    {
        if (buttonCount < UINT16_MAX) buttonCount++;

        if (buttonCount < BTN_PRESS_DEBOUNCE) {
            // button was not held long enough, do nothing
        } else if (buttonCount < BTN_PRESS_SHORT) {
            show_frame(TUNE_FRAMES, 1);
        } else if (buttonCount < BTN_PRESS_MEDIUM) {
            show_frame(TUNE_FRAMES, 3);
        } else if (buttonCount < BTN_PRESS_LONG) {
            show_frame(TUNE_FRAMES, 4);
        } else if (buttonCount >= BTN_PRESS_LONG) {
            show_frame(CLEAR, 0);
        }

        if (btn_is_released(TUNE))
        {
            if (buttonCount < BTN_PRESS_DEBOUNCE) {
                // button was not held long enough, do nothing
            } else if (buttonCount < BTN_PRESS_SHORT) {
                short_tune_release();
                break;
            } else if (buttonCount < BTN_PRESS_MEDIUM) {
                medium_tune_release();
                break;
            } else if (buttonCount < BTN_PRESS_LONG) {
                long_tune_release();
                break;
            } else if (buttonCount >= BTN_PRESS_LONG) {
                // button was held for too long, do nothing
                break;
            }
        }

        delay_ms(1);
    }

    show_frame(CLEAR, 0);
}

void func_hold(void)
{
    uint8_t FuncHoldProcessed = 0;

    while(1)
    {
        if (btn_is_pressed(CUP)) {
            FuncHoldProcessed = 1;
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

        if (btn_is_released(FUNC)) {
            break;
        }
        delay_ms(1);
    }

    if (FuncHoldProcessed == 0) mode_func();
}

void ant_hold(void)
{
    while(1)
    {
        if (btn_is_released(POWER))
        {
            break;
        }

        delay_ms(1);
    }
}

void power_hold(void)
{
    uint16_t buttonCount = 0;

    print_str_ln("power_hold");

    while(1)
    {
        if (buttonCount < UINT16_MAX) buttonCount++;

        if (buttonCount == 1000) {
            print_str_ln("going to sleep");
        }

        if (btn_is_released(POWER))
        {
            break;
        }

        delay_ms(1);
    }
}