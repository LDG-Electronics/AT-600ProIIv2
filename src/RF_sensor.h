#ifndef RF_SENSOR_H
#define	RF_SENSOR_H

/* ************************************************************************** */

typedef union {
    struct {
        uint16_t forward;
        uint16_t reverse;
        double swr;
        uint16_t frequency;
    };
} RF_power_s;

// Global RF Readings
extern RF_power_s currentRF;

// Global SWR Threshold Settings
extern uint16_t swrThresh;
extern uint8_t swrThreshIndex;

/* ************************************************************************** */

extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern void SWR_threshold_set(void);
extern void SWR_threshold_increment(void);

/* -------------------------------------------------------------------------- */

// Frequency measurement
extern uint16_t get_freq(void);
extern uint16_t get_period(void);
extern void frequency_counter_test(void);

// SWR measurement functions
extern void SWR_measure(void);
extern void SWR_average(void);
extern int8_t SWR_stable_average(void);

// Tests
void print_SWR_samples(uint8_t delta);

#endif