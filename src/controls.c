#include "includes.h"

/* ************************************************************************** */

void short_tune_release(void)
{
    toggle_bypass();
}
void medium_tune_release(void)
{
    LED_TUNE = 1;
    RADIO_OUT_PIN = 1;
    poll_for_fwd_pwr(500);
    
    memory_tune();
    
    if (tuning_flags.noMemory == 1)
    {
        full_tune();
    }
    RADIO_OUT_PIN = 0;
    LED_TUNE = 0;
    tuning_followup_animation();
}

void long_tune_release(void)
{
    LED_TUNE = 1;
    RADIO_OUT_PIN = 1;
    poll_for_fwd_pwr(500);
    
    full_tune();
    
    RADIO_OUT_PIN = 0;
    LED_TUNE = 0;
    tuning_followup_animation();
}

/* -------------------------------------------------------------------------- */

void toggle_bypass(void)
{
    relays_s relaysUndo;
    relaysUndo.all = currentRelays.all;

    if (saved_flags.inBypass == 1) {
        if (set_relays(&currentRelays) == -1) {
            currentRelays.all = relaysUndo.all;
        } else {
            blink_all(1, FAST);
        }
    } else {
        if (set_relays(&bypassRelays) == -1) {
            currentRelays.all = relaysUndo.all;
        } else {
            blink_all(3, FAST);
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
    
    if (set_relays(&currentRelays) == -1)
    {
        currentRelays.z = undo;
    }
}

void manual_store(void)
{
    uint16_t memAddr = 0;
    
    memAddr = period_to_addr(currentRF.period);
    
    // If address is within valid range, save it
    if ((memAddr != 0) && (memAddr < 2046)) {
        memAddr = (memAddr << 1) + MEM_BASE_ADDR;
        memory_store(memAddr);
        // display_animation(FUNC_TUNE_OKAY);
    } else {
        blink_all(2, FAST);
    }  
}

/* -------------------------------------------------------------------------- */

void mode_thresh(void)
{
    uint16_t modeCount = 0;
    
    blink_thresh(3, FAST);
    show_thresh();
    
    for (;;)
    {
        if (btn_is_pressed(LDN))
        {
            modeCount = 0;
            SWR_threshold_increment();
            
            show_thresh();
            blink_thresh(2, FAST);
            show_thresh();
            
            wait_for_no_buttons();
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
    uint8_t     buttons = 0;
    uint16_t    funcCounter = 0; 

    leds_scroll_up();
    funcCounter = FUNC_THRESH;
    
    while (1)
    {
        buttons = get_buttons();
        
        if (btn_is_pressed(FUNC)) {
            leds_scroll_down();
            break;
        }
        if (btn_is_pressed(CUP)) {
            // no function on this button
            break;
        }
        if (btn_is_pressed(LUP)) {
            toggle_hiloz();
            blink_HiLoZ(4, FAST);
            break;
        }
        if (btn_is_pressed(CDN)) {
            toggle_auto();
            blink_auto(4, FAST);
            break;
        }
        if (btn_is_pressed(LDN)) {
            mode_thresh();
            blink_thresh(4, FAST);
            break;
        }
        if (btn_is_pressed(TUNE)) {
            manual_store();
            break;
        }
        
        funcCounter--;
        if (funcCounter == 0) {
            leds_scroll_down();
            return;
        }
        delay_ms(1);
    }
    
    wait_for_no_buttons();
    return;
}