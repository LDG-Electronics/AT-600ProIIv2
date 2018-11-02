#ifndef _RF_SENSOR_H_
#define _RF_SENSOR_H_

#include "os/system_time.h"
#include "peripherals/adc.h"
#include <stdbool.h>
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
    float forward;
    float reverse;
    float matchQuality;
    float forwardWatts;
    float reverseWatts;
    float swr;
    uint16_t frequency;
    system_time_t lastFrequencyTime;
    system_time_t lastRFTime;
} RF_power_t;

// Global RF Readings
extern RF_power_t currentRF;

extern void print_RF_data(void);

/* ************************************************************************** */

// Setup
extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern volatile uint8_t swrThreshIndex;
extern float get_SWR_threshold(void);
extern void SWR_threshold_increment(void);

/* -------------------------------------------------------------------------- */

// SWR measurement functions
extern bool check_for_RF(void);

// measures RF and calculates SWR
extern void measure_RF(void);

/* -------------------------------------------------------------------------- */

// measures frequency
extern void measure_frequency(void);

#endif // _RF_SENSOR_H_