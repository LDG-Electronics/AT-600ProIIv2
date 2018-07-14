#ifndef RF_SENSOR_H
#define RF_SENSOR_H

/* ************************************************************************** */

// Global RF data format
typedef union {
    struct {
        uint16_t forward;
        double forwardWatts;
        uint16_t reverse;
        double reverseWatts;
        double swr;
        uint16_t frequency;
    };
} RF_power_s;

// Global RF Readings
extern RF_power_s currentRF;

// Global SWR Threshold Settings
extern double swrThresh;
extern uint8_t swrThreshIndex;

/* ************************************************************************** */

// Setup
extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern void SWR_threshold_set(void);
extern void SWR_threshold_increment(void);

/* -------------------------------------------------------------------------- */

// SWR measurement functions
extern void SWR_measure(void);
extern void SWR_average(void);
extern int8_t SWR_stable_average(void);

// Prints the current Forward, Reverse, and SWR
extern void print_current_SWR(void);
extern void print_current_SWR_ln(void);

/* ************************************************************************** */

// Tests
void print_SWR_samples(uint8_t delta);

#endif