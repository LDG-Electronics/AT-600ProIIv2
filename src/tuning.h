#ifndef _TUNING_H_
#define _TUNING_H_

#include <stdint.h>

/* ************************************************************************** */

/*  tuning_flags is a collection of read-only global status flags

    ! Read-only status is not compiler enforced: do not change these values from
    ! outside tuning.c

    These flags represent the various ways that a tuning cycle can fail.
*/

typedef union {
    struct {
        unsigned noRF : 1;
        unsigned lostRF : 1;
        unsigned badMatch : 1;
        unsigned relayError : 1;
        unsigned noMemory : 1;
    };
    uint8_t errors;
} tuning_flags_t;

extern tuning_flags_t tuning_flags;

/* ************************************************************************** */

extern void tuning_init(void);

extern void full_tune(void);
extern void memory_tune(void);

extern void tuning_followup_animation(void);

#endif