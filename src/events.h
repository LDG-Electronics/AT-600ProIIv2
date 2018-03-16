#ifndef _EVENTS_H_
#define _EVENTS_H_

/* ************************************************************************** */

extern void toggle_bypass(void);
extern void toggle_peak(void);
extern void toggle_scale(void);
extern void toggle_auto(void);
extern void toggle_hiloz(void);

extern void toggle_antenna(void);
extern void manual_store(void);

/* -------------------------------------------------------------------------- */

extern void request_memory_tune(void);
extern void request_full_tune(void);

/* -------------------------------------------------------------------------- */

// relay increment functions
extern void relays_delay_reset(void);

extern uint8_t OkToIncDec(void);
extern void capacitor_increment(void);
extern void capacitor_decrement(void);
extern void inductor_increment(void);
extern void inductor_decrement(void);

#endif