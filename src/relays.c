#include "includes.h"

/* ************************************************************************** */

/*  AT-600ProII Relay Configuration

    The YT-1200 has 7 Capacitors, 7 Inductors, and 1 HiLoZ relay.
*/

// Maximum number of relays to be switched at once: limited by PIC's maximum mA
#define RELAY_DELTA_THRESHOLD   4

// Relay delay constants, in 1 ms intervals
#define RELAY_BUS_DELAY     1
#define RELAY_COIL_DELAY    6

/* ************************************************************************** */

// Global
relays_s currentRelays;
relays_s bypassRelays;

// File
relays_s oldRelays;

// Allow for slow inc/dec, these are public because they're cleared from the main loop
uint8_t IncDecCount = 0;
uint8_t IncDecDelay = 0;

/* ************************************************************************** */

void relays_init(void)
{
    currentRelays.all = 0;
    oldRelays.all = 0;
}

/* -------------------------------------------------------------------------- */

void relay_shift_out_595_16bit(uint16_t d)
{
    uint8_t i;
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 0;
    for (i = 0; i < 16; i++)
    {
        if (d & (1 << (15 - i))) {
            RELAY_DATA_PIN = 1;
        } else {
            RELAY_DATA_PIN = 0;
        }
        delay_10us(1);
        RELAY_CLOCK_PIN = 1;
        delay_10us(1);
        RELAY_CLOCK_PIN = 0;
        delay_10us(1);
    }
    RELAY_STROBE_PIN = 0;
    delay_10us(1);
    RELAY_STROBE_PIN = 1;
    delay_10us(1);
}

/* -------------------------------------------------------------------------- */

int8_t check_if_safe(void)
{
    SWR_average();
    
    if (currentRF.forward >= HIGH_OVER_POWER) {
        #if LOG_LEVEL_RELAYS >= LOG_ERROR
        print_format(BRIGHT, MAGENTA);
        print_str_ln("relay error");
        #endif
        return (-1);
    }
    return 0;
}

/*  put_relays() takes a pointer to a relay struct and attempts to publish that
    struct to the physical relays.
    
*/
int8_t put_relays(relays_s *testRelays)
{
    if (check_if_safe() == -1) return (-1);
    
    if (saved_flags.forceAllRelays == 1)
    {
        saved_flags.forceAllRelays = 0;
        oldRelays.all = ~testRelays->all;
    }
    
    saved_flags.inBypass = 0;
    if (testRelays->all == 0) {
        saved_flags.inBypass = 1;
    }
    
    relay_shift_out_595_16bit(testRelays->all);

    oldRelays.all = testRelays->all;
    return 0;
}

/* -------------------------------------------------------------------------- */

// The next several routines are for dealing with user interface stuff
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
        if (currentRelays.caps < MAX_CAPACITORS) {
            currentRelays.caps++;
            if (put_relays(&currentRelays) == -1)
            {
                currentRelays.caps--;
            }
        } else {
            // blink_all();
        }
    }
    delay_ms(50);
}

void capacitor_decrement(void)
{
    if (OkToIncDec())
    {
        if (currentRelays.caps > MIN_CAPACITORS) {
            currentRelays.caps--;
            if (put_relays(&currentRelays) == -1)
            {
                currentRelays.caps++;
            }
        } else {
            // blink_all(4);
        }
    }
    delay_ms(50);
}

void inductor_increment(void)
{
    if (OkToIncDec())
    {
        if (currentRelays.inds < MAX_INDUCTORS) {
            currentRelays.inds++;
            if (put_relays(&currentRelays) == -1)
            {
                currentRelays.inds--;
            }
        } else {
            // blink_all(4);
        }
    }
    delay_ms(50);
}

void inductor_decrement(void)
{
    if (OkToIncDec())
    {
        if (currentRelays.inds > MIN_INDUCTORS) {
            currentRelays.inds--;
            if (put_relays(&currentRelays) == -1)
            {
                currentRelays.inds++;
            }
        } else {
            // blink_all(4);
        }
    }
    delay_ms(50);
}
