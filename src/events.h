#ifndef _EVENTS_H_
#define _EVENTS_H_

#include <stdint.h>

/* ************************************************************************** */

extern void set_bypass_on(void);
extern void set_bypass_off(void);
extern void toggle_bypass(void);

/* -------------------------------------------------------------------------- */

extern void set_peak_on(void);
extern void set_peak_off(void);
extern void toggle_peak(void);

/* -------------------------------------------------------------------------- */

extern void set_scale_high(void);
extern void set_scale_low(void);
extern void toggle_scale(void);

/* -------------------------------------------------------------------------- */

extern void set_auto_on(void);
extern void set_auto_off(void);
extern void toggle_auto(void);

/* -------------------------------------------------------------------------- */

extern void set_high_z(void);
extern void set_low_z(void);
extern void toggle_hiloz(void);

/* -------------------------------------------------------------------------- */

extern void set_antenna_one(void);
extern void set_antenna_two(void);
extern void toggle_antenna(void);

/* -------------------------------------------------------------------------- */

extern void manual_store(void);

/* -------------------------------------------------------------------------- */

extern void set_power_status(uint8_t value);
extern void toggle_power_status(void);

/* ************************************************************************** */

extern void request_memory_tune(void);
extern void request_full_tune(void);

/* -------------------------------------------------------------------------- */

#endif