#ifndef _RELAYS_H_
#define _RELAYS_H_

#include <stdint.h>

/* ************************************************************************** */

/*  AT-600ProII Relay Configuration

    The AT-600ProII has 7 Capacitors, 7 Inductors, and 1 HiLoZ relay.

    It uses 1500 Watt non-latching, 12 volt single coil relays.  The relays are
    controlled by a pair of daisy-chained TPIC6B595 Serial-in/Parallel-out
    SPI controlled shift registers.
*/

// The 1500 watt relays in this product require a long debounce time
#define RELAY_COIL_DELAY 15 // debounce time in ms

#define NUM_OF_INDUCTORS 7
#define NUM_OF_CAPACITORS 7

#define MAX_INDUCTORS 127
#define MAX_CAPACITORS 127

#define MIN_INDUCTORS 0
#define MIN_CAPACITORS 0

#define NUM_OF_ANTENNA_PORTS 2

/* ************************************************************************** */

/*  relays_t is used to simplify the storage and management of relay settings.

*/
typedef union {
    struct {
        unsigned caps : 7;
        unsigned z : 1;
        unsigned inds : 7;
        unsigned ant : 1; // Relay is wired backwards: the off position is ANT2
    };
    struct {
        uint8_t top;
        uint8_t bot;
    };
    uint16_t all;
} relays_t;

// Prints the contents of a relay struct as "(caps, inds, z)"
extern void print_relays(relays_t *relays);

/* ************************************************************************** */

// read-only: contains the most recently published relays
extern relays_t currentRelays[NUM_OF_ANTENNA_PORTS];

// returns a copy of the most recently published relays
#define read_current_relays() currentRelays[system_flags.antenna]

extern relays_t bypassRelays;
extern relays_t preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

// setup
extern void relays_init(void);

// publish new relays
extern int8_t put_relays(relays_t *relays);

#endif