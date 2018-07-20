#include "includes.h"

#include "calibration.h"
#include "frequency_counter.h"
#include "peripherals/adc.h"
#include <float.h>
#include <math.h>
#include <stdlib.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global RF Readings
RF_power_t currentRF;

/* ************************************************************************** */

void RF_sensor_init(void) {
    adc_init();
    frequency_counter_init();

    // Initialize the Global RF Readings
    currentRF.forward = 0;
    currentRF.forwardWatts = 0;
    currentRF.reverse = 0;
    currentRF.reverseWatts = 0;
    currentRF.swr = 0;
    currentRF.frequency = 0;

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

/* -------------------------------------------------------------------------- */

static double RF_sensor_compensation(uint16_t input, polynomial_s *poly) {
    double x = (double)input;

    return (poly->A * pow(x, 2)) + (poly->B * x) + poly->C;
}

/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/
static double calculate_SWR(uint16_t tempFWD, uint16_t tempREV) {
    double x = sqrt((double)tempREV / (double)tempFWD);
    return ((1.0 + x) / (1.0 - x));
}

static double calculate_SWR_by_watts(double forward, double reverse) {
    double x = sqrt(reverse / forward);
    return ((1.0 + x) / (1.0 - x));
}

/*  SWR_measure() calculates the SWR from a single sample

    This should probably only be used during development and debugging.
    It doesn't make any effort to clean or smooth the results, and therefore
    shouldn't be used to make any real decisions.
*/
void SWR_measure(void) {
    currentRF.forward = adc_measure(0);
    currentRF.reverse = adc_measure(1);
    currentRF.swr = calculate_SWR(currentRF.forward, currentRF.reverse);
}

/*  Notes on SWR_average()

    This function assumes that we've already checked that the current SWR is
    both above the low power threshold and stable.

    The key feature of this function is the loop that alternates between
    forward and reverse measurements. Because the SWR in the circuit changes
    over time, if you take n forward samples followed by n reverse samples, you
    run the risk of conditions dramatically changing between the group of
    forward measurements and the group of reverse measurements.

    In summary:
    Bad:    FFFF FFFF RRRR RRRR
    Good:   FRFR FRFR FRFR FRFR

    The optimal number of samples is influenced by several factors.
    Total measurement time = adc conversion time * (NUM_OF_SAMPLES * 2)
    12 bit adc maximum result is 4096. The maximum number of samples that fit
    into a uint16_t without data loss is 4096 * 16 = 65536.
    It's possible that too many samples will cause accuracy problems.
*/
// TODO: do Science! with the number of samples
#define NUM_OF_SWR_SAMPLES 16
void SWR_average(void) {
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    // Take our measurements
    for (uint8_t i = 0; i < NUM_OF_SWR_SAMPLES; i++) {
        tempFWD += adc_measure(0);
        tempREV += adc_measure(1);
    }

    // publish the samples and calculate the SWR
    currentRF.forward = (tempFWD / NUM_OF_SWR_SAMPLES);
    currentRF.forwardWatts = RF_sensor_compensation(currentRF.forward, &fPoly);
    // currentRF.reverse = (tempREV / NUM_OF_SWR_SAMPLES);
    currentRF.reverse = tempREV;
    currentRF.reverseWatts = RF_sensor_compensation(currentRF.reverse, &rPoly);
    currentRF.swr =
        calculate_SWR_by_watts(currentRF.forwardWatts, currentRF.reverseWatts);
}

/*  Notes on wait_for_stable_FWD()


*/
#define STABLE_RF_WINDOW 50
int8_t wait_for_stable_FWD(void) {
    uint16_t currentFWD;
    uint16_t previousFWD = adc_measure(0);
    int16_t deltaFWD = 0;
    int16_t deltaCompare = 0;

    system_time_t currentTime = systick_read();
    while (systick_elapsed_time(currentTime) <= STABLE_RF_WINDOW) {
        currentFWD = adc_measure(0);

        deltaFWD = abs((int16_t)currentFWD - (int16_t)previousFWD);
        deltaCompare = currentFWD >> 4;
        if (deltaFWD < deltaCompare)
            return 0;
        previousFWD = currentFWD;
    }
    return -1;
}

/*  Notes on SWR_stable_average()

    This function monitors the forward power and waits until the slope is flat.
*/
int8_t SWR_stable_average(void) {
    // Measure the frequency
    currentRF.frequency = get_frequency();

    // if the Frequency isn't valid then return early
    if (currentRF.frequency == 0xffff)
        return -1;

    if (wait_for_stable_FWD() == -1)
        return -1;

    SWR_average();

    return 0;
}

/* -------------------------------------------------------------------------- */

void print_current_SWR(void) {
    printf("FWD: %d, \tREV: %d, \tSWR: %f, F: %d", currentRF.forward,
           currentRF.reverse, currentRF.swr, currentRF.frequency);
}

void print_current_SWR_ln(void) {
    print_current_SWR();
    println("");
}