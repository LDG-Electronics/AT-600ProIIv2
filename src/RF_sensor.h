#ifndef RF_SENSOR_H
#define	RF_SENSOR_H

/* ************************************************************************** */

// Global RF data format
typedef union {
    struct {
        uint16_t forward;
        uint16_t reverse;
        double swr;
        uint32_t period;
        uint16_t frequency;
    };
} RF_power_s;

// Global RF Readings
extern RF_power_s currentRF;

// Global SWR Threshold Settings
extern double swrThresh;
extern uint8_t swrThreshIndex;

/* ************************************************************************** */

extern void RF_sensor_init(void);

// SWR Threshold manipulation
extern void SWR_threshold_set(void);
extern void SWR_threshold_increment(void);

/* -------------------------------------------------------------------------- */

// Frequency measurement
extern uint32_t get_period(void);

// SWR measurement functions
extern void SWR_measure(void);
extern void SWR_average(void);
extern int8_t SWR_stable_average(void);

// Prints the current Forward, Reverse, and SWR
extern void print_current_SWR(void);
extern void print_current_SWR_ln(void);

// Shell command handler
extern int shell_get_RF(int argc, char** argv);
extern void task_RF_calibration(void);

/* ************************************************************************** */

// Tests
void print_SWR_samples(uint8_t delta);

#endif