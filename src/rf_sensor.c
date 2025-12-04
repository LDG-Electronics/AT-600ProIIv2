#include "rf_sensor.h"
#include "calibration.h"
#include "os/logging.h"
#include "os/system_time.h"
#include "peripherals/adc.h"
#include "peripherals/timer.h"
#include "pins.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global RF Readings
RF_power_t currentRF;

/* ************************************************************************** */

static void clear_currentRF(void) {
    // raw readings
    currentRF.forwardVolts = 0;
    currentRF.reverseVolts = 0;
    currentRF.matchQuality = 0.0;

    // calculated values
    currentRF.forwardWatts = 0.0;
    currentRF.reverseWatts = 0.0;
    currentRF.swr = 0.0;
}

/* -------------------------------------------------------------------------- */

// re_freq.c has no header so declare init here
extern void RF_freq_init(void);

void RF_sensor_init(void) {
    adc_init();

    // Initialize the Global RF Readings
    clear_currentRF();
    currentRF.frequency = 0;
    currentRF.history = 0;
    currentRF.isPresent = false;

    // clear timestamps
    currentRF.lastMeasurementTime = 0;
    currentRF.lastCalculationTime = 0;
    currentRF.lastFrequencyTime = 0;

    RF_freq_init();

    log_register();
}

/* -------------------------------------------------------------------------- */

// SWR Threshold Settings
volatile uint8_t swrThreshIndex = 0;
const float swrThreshTable[] = {1.5, 1.7, 2.0, 2.5, 3.0};

float get_SWR_threshold(void) { return swrThreshTable[swrThreshIndex]; }

void SWR_threshold_increment(void) {
    swrThreshIndex++;
    if (swrThreshIndex == 4) {
        swrThreshIndex = 0;
    }
}

/* ************************************************************************** */
#define NUMBER_OF_SAMPLES 8
#define LOW_POWER_CUTOFF 100

// TODO: get a 1W transmitter(FT-817?)
bool check_for_RF(void) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        sum += adc_read(ADC_FWD_PIN);
    }

    uint16_t average = sum / NUMBER_OF_SAMPLES;

    // enable this for reverse power calibration
    // return true;

    if (average >= LOW_POWER_CUTOFF) {
        return true;
    }

    clear_currentRF();
    return false;
}

void poll_RF(void) {
    currentRF.history <<= 1;
    currentRF.history |= check_for_RF();

    if (currentRF.history == 0b11111111) {
        currentRF.isPresent = true;
    }

    if (currentRF.history == 0b00000000) {
        currentRF.isPresent = false;
    }
}

/* ************************************************************************** */

// #define BETA 0.025f
#define BETA 0.2f

bool wait_for_stable_RF(uint16_t timeoutDuration) {
    system_time_t startTime = get_current_time();
    float smoothFWD = 0.0f;
    float prevSmoothFWD = 0.0f;
    uint16_t iterations = 0;
    uint16_t goodSlopeCount = 0;

    while (1) {
        iterations++;
        int16_t rawFWD = adc_read(ADC_FWD_PIN);
        smoothFWD = smoothFWD - (BETA * (smoothFWD - rawFWD));

        if (fabs(prevSmoothFWD - smoothFWD) < (smoothFWD * .01f)) {
            goodSlopeCount++;
        } else {
            goodSlopeCount = 0;
            prevSmoothFWD = smoothFWD;
        }

        if (goodSlopeCount >= 10) {
            LOG_INFO({ printf("found good slope in %u iterations\r\n", iterations); });
            return true;
        }

        if (time_since(startTime) > timeoutDuration) {
            LOG_WARN({ println("timeout"); });
            return false;
        }
    }

    return false;
}

/* -------------------------------------------------------------------------- */

#define NUM_OF_SWR_SAMPLES 32
void measure_RF(void) {
    currentRF.lastMeasurementTime = get_current_time();
    uint32_t tempForward = 0;
    uint32_t tempReverse = 0;

    // Collect measurements
    for (uint8_t i = 0; i < NUM_OF_SWR_SAMPLES; i++) {
        tempForward += adc_read(ADC_FWD_PIN);
        tempReverse += adc_read(ADC_REV_PIN);
    }

    // publish the averaged forward and reverse
    currentRF.forwardVolts = (float)tempForward / NUM_OF_SWR_SAMPLES;
    currentRF.reverseVolts = (float)tempReverse / NUM_OF_SWR_SAMPLES;

    // this bitshift improves the precision of the following integer division
    tempReverse <<= 12;
    currentRF.matchQuality = (float)tempReverse / (float)tempForward;
}

bool calculate_watts_and_swr(void) {
    if (time_since(currentRF.lastMeasurementTime) > time_since(currentRF.lastCalculationTime)) {
        // There no point recalculating if the measurement hasn't changed
        return false;
    }

    currentRF.lastCalculationTime = get_current_time();
    currentRF.forwardWatts = correct_forward_power(currentRF.forwardVolts, currentRF.frequency);
    currentRF.reverseWatts = correct_reverse_power(currentRF.reverseVolts, currentRF.frequency);
    currentRF.swr = calculate_SWR_by_watts(currentRF.forwardWatts, currentRF.reverseWatts);

    return true;
}
