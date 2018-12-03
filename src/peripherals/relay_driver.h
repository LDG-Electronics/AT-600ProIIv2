#ifndef _RELAY_DRIVER_H_
#define _RELAY_DRIVER_H_

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

#define MAX_INDUCTORS ((1 << NUM_OF_INDUCTORS) - 1)
#define MAX_CAPACITORS ((1 << NUM_OF_CAPACITORS) - 1)

#define MIN_INDUCTORS 0
#define MIN_CAPACITORS 0

#define NUM_OF_ANTENNA_PORTS 2

/*  packed_relays_t is used to simplify the translation of software
    representation of relay information into a packed bit representation that's
    useful for communicating with the hardware.
*/
typedef union {
    struct {
        unsigned caps : NUM_OF_CAPACITORS;
        unsigned z : 1;
        unsigned inds : NUM_OF_INDUCTORS;
        unsigned ant : 1; // Relay is wired backwards: the off position is ANT2
    };
    struct {
        uint8_t top;
        uint8_t bot;
    };
    uint16_t bits;
} packed_relays_t;

/* ************************************************************************** */

// setup
extern void relay_driver_init(void);

/*  publish_relays() writes a packed relay object out to the hardware

    This sends a bit pattern representing the desired relays to the relay driver
    shift registers via bitbang SPI.

    Additionally, are unstable for a certain period of time. This function has
    a blocking delay that prevents the rest of the system from doing things
    during that debounce time.
*/
extern void publish_relays(packed_relays_t *relayBits);

/* ************************************************************************** */

// Prints the contents of a packed_relays_t as "(<caps>, <inds>, <z>, <ant>)"
extern void print_relay_bits(packed_relays_t *relayBits);

#endif // _RELAY_DRIVER_H_