#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#include <stdint.h>

/* ************************************************************************** */
/*  system_flags_t contains information that should be retained after cycling
    the power.  The contents are saved to EEPROM whenever a setting is changed,
    and are read from EEPROM during boot.
    
    If possible, keep this struct under 8 bits in size, because we'll need to
    store it in EEPROM, and it would be a waste to use 2 bytes of storage for
    9 or 10 bits of flags.
*/
typedef struct {
    unsigned ant1Bypass : 1; // Are we in bypass mode?
    unsigned ant2Bypass : 1; // Are we in bypass mode?
    unsigned antenna : 1;    // Which antenna is selected
    unsigned autoMode : 1;   // Are we in semi (0) or full (1) auto mode?
    unsigned peakMode : 1;   //
    unsigned scaleMode : 1;  //
    unsigned powerStatus : 1;
} system_flags_t;

// globally accessible system status flags
extern system_flags_t systemFlags;

/* ************************************************************************** */

/*  This is used to access the current bypass status using the same idiom as
    accessing currentRelays:

    currentRelays[systemFlags.antenna]
    bypassStatus[systemFlags.antenna]
    etc
*/
extern uint8_t bypassStatus[2];

/* ************************************************************************** */

// setup
extern void flags_init(void);

// restore most recently saved system settings from EEPROM
extern void load_flags(void);

// save current system settings to EEPROM
extern void save_flags(void);

#endif