#ifndef _TUNING_H_
#define _TUNING_H_

#include <stdint.h>

/* ************************************************************************** */

typedef union {
    struct {
        unsigned noRF : 1;
        unsigned lostRF : 1;
        unsigned badMatch : 1;
        unsigned relayError : 1;
        unsigned noMemory : 1;
    };
    uint8_t any;
} tuning_errors_t;

/* ************************************************************************** */

// setup
extern void tuning_init(void);

/* -------------------------------------------------------------------------- */
/*  Tuning

*/

// attempts to tune, without using memories
extern tuning_errors_t full_tune(void);

// attempts to look up a stored memory that matches the current frequency
extern tuning_errors_t memory_tune(void);

/* -------------------------------------------------------------------------- */

// 
extern void tuning_followup_animation(tuning_errors_t errors);

#endif