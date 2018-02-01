#ifndef RF_SENSOR_H
#define	RF_SENSOR_H

/* ************************************************************************** */

typedef union {
    struct {
        uint16_t forward;
        uint16_t reverse;
        uint16_t swr;
        uint16_t period;
    };
} RF_power_s;

// Global RF Readings
extern RF_power_s currentRF;

/* ************************************************************************** */

extern void RF_sensor_init(void);

/* -------------------------------------------------------------------------- */

// Frequency measurement
extern uint16_t get_freq(void);
extern uint16_t get_period(void);

#endif
