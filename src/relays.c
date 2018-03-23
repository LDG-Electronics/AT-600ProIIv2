#include "includes.h"

/* ************************************************************************** */

/*  AT-600ProII Relay Configuration

    The AT-600ProII has 7 Capacitors, 7 Inductors, and 1 HiLoZ relay.

    It uses 1500 Watt non-latching, 12 volt single coil relays.  The relays are
    controlled by a pair of daisy-chained TPIC6B595 Serial-in/Parallel-out
    SPI controlled shift registers.
*/

// Relay delay constants, in 1 ms intervals
#define RELAY_COIL_DELAY 15

/* ************************************************************************** */

// Global
relays_s currentRelays[NUM_OF_ANTENNA_PORTS];
relays_s bypassRelays; //TODO: needs to be const, and permanently equal {0, 0, 0}
relays_s preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

void relays_init(void)
{
    // Initialize relay structs
    currentRelays[0].all = 0;
    currentRelays[0].ant = 0;
    currentRelays[1].all = 0;
    currentRelays[1].ant = 1;
    bypassRelays.all = 0;
    preBypassRelays[0].all = 0;
    preBypassRelays[0].ant = 0;
    preBypassRelays[1].all = 0;
    preBypassRelays[1].ant = 1;

    // 
    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 1;
    RELAY_DATA_PIN = 1;
}

/* -------------------------------------------------------------------------- */

void publish_relays(uint16_t word)
{
    uint8_t i;

    RELAY_STROBE_PIN = 1;
    RELAY_CLOCK_PIN = 0;

    for (i = 0; i < 16; i++)
    {
        if (word & (1 << (15 - i))) {
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
    // if (check_if_safe() == -1) return (-1);
    
    bypassStatus[system_flags.antenna] = 0;
    if ((testRelays->caps == 0) && 
        (testRelays->z == 0) && 
        (testRelays->inds == 0)) {
            
        bypassStatus[system_flags.antenna] = 1;
    }

    update_bypass_led();
    
    publish_relays(testRelays->all);

    delay_ms(RELAY_COIL_DELAY);

    return 0;
}

/* -------------------------------------------------------------------------- */

// The next several routines are for dealing with user interface stuff
