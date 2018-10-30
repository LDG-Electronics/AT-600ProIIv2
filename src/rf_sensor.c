#include "rf_sensor.h"
#include "calibration.h"
#include "frequency_counter.h"
#include "os/log_macros.h"
#include "os/system_time.h"
#include "peripherals/adc.h"
#include <float.h>
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
    currentRF.forwardADC = 0;
    currentRF.reverseADC = 0;
    currentRF.swrADC = 0.0;
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
const double swrThreshTable[] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};

double get_SWR_threshold(void) { return swrThreshTable[swrThreshIndex]; }

void SWR_threshold_increment(void) {
    swrThreshIndex++;
    if (swrThreshIndex == 4) {
        swrThreshIndex = 0;
    }
}

/* ************************************************************************** */

bool check_for_RF(void) {
    uint16_t sum = 0;
    for (uint8_t i = 0; i < 8; i++) {
        sum += adc_single_sample(0);
    }

    uint16_t average = sum / 8;

    if (average >= 200) {
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */

static double RF_sensor_compensation(uint16_t input, const polynomial_t *poly) {
    double x = (double)input;

    double result = (poly->A * pow(x, 2)) + (poly->B * x) + poly->C;

    if (result < 0) {
        result = 0;
    }

    return result;
}

/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/

static double calculate_SWR_by_watts(double forward, double reverse) {
    double x = sqrt(reverse / forward);
    return ((1.0 + x) / (1.0 - x));
}

void measure_RF(void) {
    currentRF.lastRFTime = systick_read();
    currentRF.forward = adc_read(0);
    currentRF.reverse = adc_read(1);

    // if (currentRF.forward.discardedSamples > 0) {
    //     clear_currentRF();
    //     return;
    // }

    currentRF.matchQuality = currentRF.reverse.value / currentRF.forward.value;
    currentRF.swr = currentRF.matchQuality;
    currentRF.forwardADC = (uint16_t)currentRF.forward.value;

    uint8_t bandIndex = decode_frequency_to_band_index(currentRF.frequency);
    currentRF.forwardWatts = RF_sensor_compensation(
        currentRF.forwardADC, &calibrationTable[0][bandIndex]);
}

// TODO: I don't think this is returning early if SWR is present

#define STABLE_RF_WINDOW 50
int8_t wait_for_stable_FWD(void) {
    uint16_t currentFWD;
    uint16_t previousFWD = adc_single_sample(0);
    int16_t deltaFWD = 0;
    int16_t deltaCompare = 0;

    system_time_t currentTime = systick_read();
    while (systick_elapsed_time(currentTime) <= STABLE_RF_WINDOW) {
        currentFWD = adc_single_sample(0);

        deltaFWD = abs((int16_t)currentFWD - (int16_t)previousFWD);
        deltaCompare = currentFWD >> 4;
        if (deltaFWD < deltaCompare) {
            return 0;
        }
        previousFWD = currentFWD;
    }
    return -1;
}

/*  Notes on SWR_stable_average()

    This function monitors the forward power and waits until the slope is flat.
*/
int8_t SWR_stable_average(void) {
    get_frequency();

    // if the Frequency isn't valid then return early
    if (currentRF.frequency == 0xffff) {
        clear_currentRF();
        return -1;
    }

    if (wait_for_stable_FWD() == -1) {
        clear_currentRF();
        return -1;
    }

    measure_RF();

    if (currentRF.forwardADC < 8) {
        clear_currentRF();
    }

    return 0;
}