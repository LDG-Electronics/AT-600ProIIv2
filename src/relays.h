#ifndef _RELAYS_H_
#define _RELAYS_H_

/* ************************************************************************** */

/*  AT-600ProII Relay Configuration

    7 Capacitors
    7 Inductors
    1 HiLoZ relay
    
    1 Antenna relay    
*/

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
        unsigned ant : 1;
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

// relay increment functions
extern void relays_delay_reset(void);

extern uint8_t OkToIncDec(void);
extern void capacitor_increment(void);
extern void capacitor_decrement(void);
extern void inductor_increment(void);
extern void inductor_decrement(void);

#endif /* _RELAYS_H_ */
