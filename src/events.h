#ifndef _EVENTS_H_
#define _EVENTS_H_

/* ************************************************************************** */

extern void set_bypass(uint8_t value); // 0 = bypass, 1 = not
extern void toggle_bypass(void);

extern void set_peak(uint8_t value); // 0 = average, 1 = peak
extern void toggle_peak(void);

extern void set_scale(uint8_t value); // 0 = low scale, 1 = high scale
extern void toggle_scale(void);

extern void set_auto(uint8_t value); // 0 = semi, 1 = auto
extern void toggle_auto(void);

extern void set_hiloz(uint8_t value); // 0 = LoZ, 1 = HiZ
extern void toggle_hiloz(void);

extern void set_antenna(uint8_t value); // 0 = Ant 1, 1 = Ant 2
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

#endif