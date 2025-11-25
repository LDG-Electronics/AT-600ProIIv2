#ifndef _TUNING_UTILS_H_
#define _TUNING_UTILS_H_

#include "relays.h"
#include <stdint.h>

/* ************************************************************************** */

// setup
extern void tuning_utils_init(void);

/* ************************************************************************** */

// stores the number of solutions tried
extern uint16_t comparisonCount;

// resets the solution counter, call this at the beginning of a tune cycle
extern void reset_solution_count(void);

// prints how many solutions have been tried during this tune cycle
extern void print_comparison_count(void);

/* ************************************************************************** */
/*  Tuning Error Struct

*/

typedef union {
    struct {
        unsigned noFreq : 1;
        unsigned noRF : 1;
        unsigned lostRF : 1;
        unsigned badMatch : 1;
        unsigned relayError : 1;
        unsigned noMemory : 1;
        unsigned timeout : 1;
    };
    uint8_t any;
} tuning_errors_t;

// returns a correctly initialized tuning_errors_t object
extern tuning_errors_t no_errors(void);

// evaluate the tuning errors and perform the appropriate response
extern void tuning_followup_animation(tuning_errors_t errors);

/* ************************************************************************** */
/*  Tuning Match Struct

*/

typedef struct {
    uint16_t attemptNumber;
    relays_t relays;
    float forward;
    float reverse;
    float matchQuality;
    uint16_t frequency;
} match_t;

// returns a correctly initialized match_t object
extern match_t new_match(void);

// prints a match_t object with proper formatting
extern void print_match(match_t *match);

//
extern match_t select_best_match(match_t matchA, match_t matchB);

//
extern match_t compare_matches(tuning_errors_t *errors, relays_t relays, match_t bestMatch);

/* ************************************************************************** */

//
extern uint8_t calculate_max_capacitor(uint16_t frequency);
//
extern uint8_t calculate_max_inductor(uint16_t frequency);

#endif // _TUNING_UTILS_H_