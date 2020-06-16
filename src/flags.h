#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

#include "relays.h"
#include <stdint.h>

/* ************************************************************************** */

typedef struct {
    uint8_t bypassStatus[NUM_OF_ANTENNA_PORTS]; // 1 is bypass, 0 is.. not?
    uint8_t antenna;                            // 1 is Ant1, 0 is Ant2
    uint8_t autoMode;                           // 1 is auto, 0 is semi
    uint8_t peakMode;                           // 1 is peak, 0 is average
    uint8_t scaleMode;                          // 1 is full scale
    uint8_t powerStatus;                        // 1 is on, 0 is off
} system_flags_t;

// globally accessible system status flags
extern system_flags_t systemFlags;

/* ************************************************************************** */

// setup
extern void flags_init(void);

// restore most recently saved system settings from EEPROM
extern void load_flags(void);

// save current system settings to EEPROM
extern void save_flags(void);

#endif