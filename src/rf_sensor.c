#include "rf_sensor.h"
#include "calibration.h"
#include "frequency_counter.h"
#include "os/log_macros.h"
#include "os/system_time.h"
#include "peripherals/adc.h"
#include <math.h>
#include <stdlib.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global RF Readings
RF_power_t currentRF;

/* ************************************************************************** */

void clear_currentRF(void) {
    currentRF.forward.value = 0;
    currentRF.forward.discardedSamples = 0;
    currentRF.reverse.value = 0;
    currentRF.reverse.discardedSamples = 0;
    currentRF.matchQuality = 0.0;
    currentRF.forwardWatts = 0.0;
    currentRF.reverseWatts = 0.0;
    currentRF.swr = 0.0;
    currentRF.frequency = 0xffff;
}

void RF_sensor_init(void) {
    adc_init();
    frequency_counter_init();

    // Initialize the Global RF Readings
    clear_currentRF();
    currentRF.lastFrequencyTime = 0;
    currentRF.lastRFTime = 0;

    log_register();
}

/* -------------------------------------------------------------------------- */

// SWR Threshold Settings
volatile uint8_t swrThreshIndex = 0;
const float swrThreshTable[] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};

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
        sum += adc_single_sample(0);
    }

    uint16_t average = sum / NUMBER_OF_SAMPLES;

    if (average >= LOW_POWER_CUTOFF) {
        return true;
    }

    clear_currentRF();
    return false;
}

/* -------------------------------------------------------------------------- */

static float RF_sensor_compensation(float x, const polynomial_t *poly) {
    return (poly->A * pow(x, 2)) + (poly->B * x) + poly->C;
}

/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/

static float calculate_SWR_by_watts(float forward, float reverse) {
    float x = sqrt(reverse / forward);
    return ((1.0 + x) / (1.0 - x));
}

void measure_RF(void) {
    currentRF.lastRFTime = get_current_time();
    currentRF.forward = adc_read(0);
    currentRF.reverse = adc_read(1);

    // if (currentRF.forward.discardedSamples > 0) {
    //     clear_currentRF();
    //     return;
    // }

    currentRF.matchQuality = currentRF.reverse.value / currentRF.forward.value;
    currentRF.swr = currentRF.matchQuality;

    uint8_t bandIndex = decode_frequency_to_band_index(currentRF.frequency);
    currentRF.forwardWatts = RF_sensor_compensation(
        currentRF.forward.value, &calibrationTable[0][bandIndex]);
}