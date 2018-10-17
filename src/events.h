#ifndef _EVENTS_H_
#define _EVENTS_H_

/* ************************************************************************** */

extern void set_bypass_on(void);
extern void set_bypass_off(void);
extern void toggle_bypass(void);

extern void set_peak_on(void);
extern void set_peak_off(void);
extern void toggle_peak(void);

extern void set_scale_high(void);
extern void set_scale_low(void);
extern void toggle_scale(void);

extern void set_auto_on(void);
extern void set_auto_off(void);
extern void toggle_auto(void);

extern void set_high_z(void);
extern void set_low_z(void);
extern void toggle_hiloz(void);

extern void set_antenna_one(void);
extern void set_antenna_two(void);
extern void toggle_antenna(void);

extern void manual_store(void);

/* -------------------------------------------------------------------------- */

extern void request_memory_tune(void);
extern void request_full_tune(void);

/* -------------------------------------------------------------------------- */

// relay increment functions
extern void capacitor_increment(void);
extern void capacitor_decrement(void);
extern void inductor_increment(void);
extern void inductor_decrement(void);

/* -------------------------------------------------------------------------- */

#endif