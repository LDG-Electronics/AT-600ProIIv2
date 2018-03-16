#include "includes.h"

/* ************************************************************************** */

// Allow for slow inc/dec, these are public because they're cleared from the main loop
uint8_t IncDecCount = 0;
uint8_t IncDecDelay = 0;

/* -------------------------------------------------------------------------- */

void toggle_bypass(void)
{
    relays_s undoRelays;
    undoRelays.all = currentRelays[system_flags.antenna].all;

    if (bypassStatus[system_flags.antenna] == 1) {
        currentRelays[system_flags.antenna].all = preBypassRelays[system_flags.antenna].all;
        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].all = undoRelays.all;
        } else {
            // show_relays();
            // delay_ms(250);
            // display_clear();
        }
    } else {        
        preBypassRelays[system_flags.antenna].all = currentRelays[system_flags.antenna].all;
        currentRelays[system_flags.antenna].caps = 0;
        currentRelays[system_flags.antenna].inds = 0;
        currentRelays[system_flags.antenna].z = 0;
        if (put_relays(&bypassRelays) == -1) {
            currentRelays[system_flags.antenna].all = undoRelays.all;
        } else {
            // repeat_animation(&blink_both_bars, 3);
        }
    }
}

void toggle_peak(void)
{
    system_flags.peakMode = !system_flags.peakMode;
}

void toggle_scale(void)
{
    system_flags.Scale100W = !system_flags.Scale100W;
}

void toggle_auto(void)
{
    system_flags.autoMode = !system_flags.autoMode;
}

void toggle_hiloz(void)
{
    uint8_t undo = currentRelays[system_flags.antenna].z;

    currentRelays[system_flags.antenna].z = !currentRelays[system_flags.antenna].z;
    
    if (put_relays(&currentRelays[system_flags.antenna]) == -1)
    {
        currentRelays[system_flags.antenna].z = undo;
    }
}

void toggle_antenna(void)
{
    uint8_t undo = system_flags.antenna;

    system_flags.antenna = !system_flags.antenna;
    
    if (put_relays(&currentRelays[system_flags.antenna]) == -1)
    {
        system_flags.antenna = undo;
    }
    
    update_antenna_led();
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

void request_memory_tune(void)
{
    // poll_for_fwd_pwr(500);
    
    memory_tune();
    
    if (tuning_flags.noMemory == 1)
    {
        full_tune();
    }
    tuning_followup_animation();
}

void request_full_tune(void)
{
    // poll_for_fwd_pwr(500);
    
    full_tune();

    tuning_followup_animation();
}

/* -------------------------------------------------------------------------- */

void relays_delay_reset(void)
{
    IncDecCount = 0;
    IncDecDelay = 0;
}

uint8_t OkToIncDec(void)
{
    if (IncDecCount < 5)  {
        if (IncDecDelay == 0) {
            IncDecDelay++;
            return (1);
        } else {
            IncDecDelay++;
        }
        
        if (IncDecDelay > 5)
        {
            IncDecDelay = 0;
            IncDecCount++;
        }
        return(0);
    } else {
        return (1);
    }
}

void capacitor_increment(void)
{
    if (OkToIncDec())
    {
        if (currentRelays[system_flags.antenna].caps < MAX_CAPACITORS) {
            currentRelays[system_flags.antenna].caps++;
            if (put_relays(&currentRelays[system_flags.antenna]) == -1)
            {
                currentRelays[system_flags.antenna].caps--;
            }
            show_relays();
        } else {
            repeat_animation(&blink_power_bar, 3);
        }
    }
    delay_ms(50);
}

void capacitor_decrement(void)
{
    if (OkToIncDec())
    {
        if (currentRelays[system_flags.antenna].caps > MIN_CAPACITORS) {
            currentRelays[system_flags.antenna].caps--;
            if (put_relays(&currentRelays[system_flags.antenna]) == -1)
            {
                currentRelays[system_flags.antenna].caps++;
            }
            show_relays();
        } else {
            repeat_animation(&blink_power_bar, 3);
        }
    }
    delay_ms(50);
}

void inductor_increment(void)
{
    if (OkToIncDec())
    {
        if (currentRelays[system_flags.antenna].inds < MAX_INDUCTORS) {
            currentRelays[system_flags.antenna].inds++;
            if (put_relays(&currentRelays[system_flags.antenna]) == -1)
            {
                currentRelays[system_flags.antenna].inds--;
            }
            show_relays();
        } else {
            repeat_animation(&blink_swr_bar, 3);
        }
    }
    delay_ms(50);
}

void inductor_decrement(void)
{
    if (OkToIncDec())
    {
        if (currentRelays[system_flags.antenna].inds > MIN_INDUCTORS) {
            currentRelays[system_flags.antenna].inds--;
            if (put_relays(&currentRelays[system_flags.antenna]) == -1)
            {
                currentRelays[system_flags.antenna].inds++;
            }
            show_relays();
        } else {
            repeat_animation(&blink_swr_bar, 3);
        }
    }
    delay_ms(50);
}
