#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

/* ************************************************************************** */

/*  Globally visible struct definitions.
    
    Because C's entire system of includes and headers is dumb, this file is the
    easiest way to make certain that every part of the program will have access
    to the definition of any structs that are used.
    
    Something something translation units, something something object linking.
*/

/*  system_flags_s contains information that should be retained after cycling the
    power.  The contents are saved to EEPROM whenever a setting is changed, and
    are read from EEPROM during boot.
    
    If possible, keep this struct under 8 bits in size, because we have to
    access if from all over the place.
*/
typedef union {
    struct {
        unsigned ant1Bypass : 1; // Are we in bypass mode?
        unsigned ant2Bypass : 1; // Are we in bypass mode?
        unsigned antenna : 1; // Which antenna is selected 
        unsigned autoMode : 1; // Are we in semi (0) or full (1) auto mode?
        unsigned peakMode : 1; // 
        unsigned Scale100W : 1; // 
    };
    uint8_t flags;
} system_flags_s;

/* ************************************************************************** */

// Global flags 
extern system_flags_s system_flags;

extern uint8_t bypassStatus[2];

/* ************************************************************************** */

extern void flags_init(void);

extern void save_flags(void);
extern void load_flags(void);

#endif
