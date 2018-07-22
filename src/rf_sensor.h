#ifndef RF_SENSOR_H
#define RF_SENSOR_H

#include <stdint.h>

/* ************************************************************************** */

#define SWR1_0 0
#define SWR1_1 1
#define SWR1_3 7
#define SWR1_5 10
#define SWR1_7 20
#define SWR2_0 40
#define SWR2_5 75
#define SWR3_0 100
#define SWR3_5 120

// Global RF data format
typedef struct {
    uint16_t forward;
    double forwardWatts;
    uint16_t reverse;
    double reverseWatts;
    double swr;
    uint16_t frequency;

} RF_power_t;

// Global RF Readings
extern RF_power_t currentRF;

/* ************************************************************************** */

// Setup
extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern volatile uint8_t swrThreshIndex;
extern double get_SWR_threshold(void);
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