#ifndef _RF_SENSOR_H_
#define _RF_SENSOR_H_

#include "os/system_time.h"
#include "peripherals/adc.h"
#include <stdbool.h>
#include <stdint.h>

/* ************************************************************************** */

typedef struct {
    float forward;      // forward power in millivolts
    float reverse;      // reverse power in millivolts
    float matchQuality; // psuedo-SWR, calcuated from from raw forward/reverse
    system_time_t lastMeasurementTime;
    float forwardWatts; // forward power in watts
    float reverseWatts; // reverse power in watts
    float swr;          // SWR, calculated from corrected wattages
    system_time_t lastCalculationTime;
    uint16_t frequency; // frequency in KHz
    system_time_t lastFrequencyTime;
} RF_power_t;

// read-only: contains the most recent RF measurements
extern RF_power_t currentRF;

/* ************************************************************************** */

// Setup
extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern volatile uint8_t swrThreshIndex;

// returns the current SWR threshold, using the swrThreshIndex
extern float get_SWR_threshold(void);

// increase the threshold by one slot
extern void SWR_threshold_increment(void);

/* -------------------------------------------------------------------------- */

// returns true is RF is detected, false if not
extern bool check_for_RF(void);

// tries to detect RF for timeoutDuration mS
extern bool wait_for_stable_RF(uint16_t timeoutDuration);

// measures forward & reverse, and calculates matchQuality
extern void measure_RF(void);

// calculates forwardWatts & reverseWatts, and uses those to calculate SWR
extern bool calculate_watts_and_swr(void);

/* -------------------------------------------------------------------------- */

// measures frequency
extern void measure_frequency(void);

#endif // _RF_SENSOR_H_