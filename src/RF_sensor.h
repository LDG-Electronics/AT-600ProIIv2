#ifndef RF_SENSOR_H
#define	RF_SENSOR_H

/* ************************************************************************** */

// Swr computation result definitions. Definitely hardware dependent!
#define SWR1_0 0
#define SWR1_1 1
#define SWR1_3 7
#define SWR1_5 10
#define SWR1_7 20
#define SWR2_0 40
#define SWR2_5 75
#define SWR3_0 100
#define SWR3_5 120

/* ************************************************************************** */

// This is a little above the 125W values below, and will hopefully not trigger
// a false positive
#define HIGH_OVER_POWER     235

// 125W on Alpha Power 4510A, from Kenwood TS-480X into 50ohm dummy load
// Updated 3/20/17 to adjust for new SWR math
#define OVER_POWER_2MHZ     171
#define OVER_POWER_3MHZ     202
#define OVER_POWER_5MHZ     212
#define OVER_POWER_7MHZ     215
#define OVER_POWER_10MHZ    218
#define OVER_POWER_14MHZ    220
#define OVER_POWER_18MHZ    216
#define OVER_POWER_21MHZ    216
#define OVER_POWER_24MHZ    218
#define OVER_POWER_28MHZ    215

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

// SWR measurement functions
extern void SWR_measure(void);
extern void SWR_average(void);
extern int8_t SWR_stable_average(void);

// Tests
void take_SWR_samples(void);

#endif