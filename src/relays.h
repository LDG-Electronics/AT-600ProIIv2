#ifndef _RELAYS_H_
#define _RELAYS_H_

#include "peripherals/relay_driver.h"
#include <stdint.h>

/* ************************************************************************** */
/*  AT-600ProII Relay Configuration

    The AT-600ProII has 7 Capacitors, 7 Inductors, and 1 HiLoZ relay.

    It uses 1500 Watt non-latching, 12 volt single coil relays.
*/

/* ************************************************************************** */
/*  relays_t is used to simplify the storage and management of relay settings.

*/
typedef union {
    struct {
        uint8_t caps;
        uint8_t inds;
        uint8_t z;
        uint8_t ant; // Relay is wired backwards: the off position is ANT2
    };
    uint32_t all;
} relays_t;

/* ************************************************************************** */

// read-only: contains the most recently published relays
extern relays_t currentRelays[NUM_OF_ANTENNA_PORTS];

// returns a copy of the most recently published relays
#define read_current_relays() currentRelays[system_flags.antenna]

// read-only: should only ever contain {0,0,0}
extern relays_t bypassRelays;

// NOT read-only:
extern relays_t preBypassRelays[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

// setup
extern void relays_init(void);

extern packed_relays_t pack_relays(relays_t *relays);
extern relays_t unpack_relays(packed_relays_t *relayBits);

// publish new relays
extern int8_t put_relays(relays_t *relays);

/* ************************************************************************** */

// Prints the contents of a relays_t as "(<caps>, <inds>, <z>, <ant>)"
extern void print_relays(relays_t *relays);

#endif // _RELAYS_H_