#ifndef _TUNING_H_
#define _TUNING_H_

/* ************************************************************************** */

// TODO: describe this shit

typedef union {
    struct {
        unsigned noRF : 1;
        unsigned lostRF : 1;
        unsigned badMatch : 1;
        unsigned relayError : 1;
        unsigned noMemory : 1;
    };
    uint8_t errors;
}tuning_flags_s;

extern tuning_flags_s tuning_flags;

/* ************************************************************************** */

extern void full_tune (void);
extern void memory_tune (void);

extern void tuning_followup_animation(void);

#endif

