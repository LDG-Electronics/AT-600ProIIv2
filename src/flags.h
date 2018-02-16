#ifndef _PARAMETERS_H_
#define _PARAMETERS_H_

/* ************************************************************************** */

/*  Globally visible struct definitions.
    
    Because C's entire system of includes and headers is dumb, this file is the
    easiest way to make certain that every part of the program will have access
    to the definition of any structs that are used.
    
    Something something translation units, something something object linking.
*/

/*  saved_flags_s contains information that should be retained after cycling the
    power.  The contents are saved to EEPROM whenever a setting is changed, and
    are read from EEPROM during boot.
    
    If possible, keep this struct under 8 bits in size, because we have to
    access if from all over the place.
*/
typedef union {
    struct {
        unsigned forceAllRelays : 1; // Set if first time into put_relays,
                                     // to force all relay set
        unsigned inBypass : 1; // Are we in bypass mode?
        unsigned AutoMode : 1; // Are we in semi (0)  or full (1) auto mode?
        unsigned Antenna : 1; // Which antenna is selected 
        unsigned PeakOn : 1; // 
        unsigned Scale100W : 1; // 
    };
    uint8_t flags;
} saved_flags_s;

/*  unsaved_flags_s contains information that does NOT need to be retained after
    a power cycle.
    
    If possible, keep this struct under 8 bits in size, because we have to
    access if from all over the place.
*/
typedef union {
    struct {
        unsigned allowAutoTune : 1; // 
        unsigned FuncIsHeld : 1; // Function button held in
        unsigned TuneIsHeld : 1; // Tune button held in
        unsigned FuncHoldProcessed : 1; // Processed a func-hold
    };
    uint8_t flags;
} unsaved_flags_s;

/* ************************************************************************** */

// Global structs 
extern saved_flags_s saved_flags;
extern unsaved_flags_s unsaved_flags;

/* ************************************************************************** */

extern void store_flags(void);
extern void retrieve_flags(void);

#endif
