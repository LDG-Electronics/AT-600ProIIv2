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

/*  relays_s is used to simplify the storage and management of relay settings.

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
} relays_s;

/* ************************************************************************** */

extern relays_s currentRelays[NUM_OF_ANTENNA_PORTS];
extern relays_s bypassRelays;
extern relays_s preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

extern void relays_init(void);
extern int8_t put_relays(relays_s *testRelays);

// Prints the contents of a relay struct as "(caps, inds, z)"
extern void print_relays(relays_s *relays);
extern void print_relays_ln(relays_s *relays);

#endif