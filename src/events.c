#include "includes.h"

/* ************************************************************************** */

void set_bypass(uint8_t value)
{

}

void toggle_bypass(void)
{
    relays_s undoRelays;
    undoRelays.all = currentRelays[system_flags.antenna].all;

    if (bypassStatus[system_flags.antenna] == 1) {
        currentRelays[system_flags.antenna].all = preBypassRelays[system_flags.antenna].all;

        if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
            currentRelays[system_flags.antenna].all = undoRelays.all;
        }
    } else {        
        preBypassRelays[system_flags.antenna].all = currentRelays[system_flags.antenna].all;
        currentRelays[system_flags.antenna].caps = 0;
        currentRelays[system_flags.antenna].inds = 0;
        currentRelays[system_flags.antenna].z = 0;

        if (put_relays(&bypassRelays) == -1) {
            currentRelays[system_flags.antenna].all = undoRelays.all;
        }
    }
}

void set_peak(uint8_t value)
{
    system_flags.peakMode = value;
}

void toggle_peak(void)
{
    system_flags.peakMode = !system_flags.peakMode;
}

void set_scale(uint8_t value)
{
    system_flags.Scale100W = value;
}

void toggle_scale(void)
{
    system_flags.Scale100W = !system_flags.Scale100W;
}

void set_auto(uint8_t value)
{
    system_flags.autoMode = value;
}

void toggle_auto(void)
{
    system_flags.autoMode = !system_flags.autoMode;
}

void set_hiloz(uint8_t value)
{
    uint8_t undo = currentRelays[system_flags.antenna].z;

    currentRelays[system_flags.antenna].z = value;
    
    if (put_relays(&currentRelays[system_flags.antenna]) == -1)
    {
        currentRelays[system_flags.antenna].z = undo;
    }
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

void set_antenna(uint8_t value)
{
    uint8_t undo = system_flags.antenna;

    system_flags.antenna = value;
    
    if (put_relays(&currentRelays[system_flags.antenna]) == -1)
    {
        system_flags.antenna = undo;
    }

    update_antenna_led();
}

void toggle_antenna(void)
{
    set_antenna(!system_flags.antenna);
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
    RADIO_CMD_PIN = 1;
    // poll_for_fwd_pwr(500);
    
    memory_tune();
    
    if (tuning_flags.noMemory == 1)
    {
        full_tune();
    }

    RADIO_CMD_PIN = 0;
    tuning_followup_animation();
}

void request_full_tune(void)
{
    RADIO_CMD_PIN = 1;
    // poll_for_fwd_pwr(500);
    
    full_tune();

    RADIO_CMD_PIN = 0;
    tuning_followup_animation();
}

/* -------------------------------------------------------------------------- */

void capacitor_increment(void)
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

void capacitor_decrement(void)
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

void inductor_increment(void)
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

void inductor_decrement(void)
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
