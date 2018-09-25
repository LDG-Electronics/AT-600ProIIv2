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
    currentRF.forwardADC = 0;
    currentRF.forwardWatts = 0.0;
    currentRF.reverseADC = 0;
    currentRF.reverseWatts = 0.0;
    currentRF.swr = 0.0;
    currentRF.frequency = 0xffffff;
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
static double calculate_SWR(uint16_t tempFWD, uint16_t tempREV) {
    double x = sqrt((double)tempREV / (double)tempFWD);
    return ((1.0 + x) / (1.0 - x));
}

static double calculate_SWR_by_watts(double forward, double reverse) {
    double x = sqrt(reverse / forward);
    return ((1.0 + x) / (1.0 - x));
}

static double calc_vswr(double reflectionCoefficient) {
    double x = fabs(reflectionCoefficient);
    return ((1.0 + x) / (1.0 - x));
}

uint16_t get_forward_sample(void) {
    // beep
    return adc_measure(0);
}

#define NUM_OF_REV_SAMPLES 16
uint16_t get_reverse_sample(void) {
    uint16_t samplebuffer[NUM_OF_REV_SAMPLES];
    uint16_t totalSamples = 0;
    uint8_t samples = 0;

    // grab a shitload of samples
    // shitload = 16
    // metric shitton = 128
    while (samples < NUM_OF_REV_SAMPLES) {
        uint16_t tempREV = adc_measure(1);
        totalSamples++;
        if (tempREV != 0) {
            samplebuffer[samples++] = tempREV;
        }
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        sum += samplebuffer[i];
    }

    return sum / NUM_OF_REV_SAMPLES;
}

uint16_t get_forward_sample_test(void) {
    uint16_t samplebuffer[NUM_OF_REV_SAMPLES];
    // clear buffer
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        samplebuffer[i] = 0;
    }

    us_stopwatch_begin();

    // grab a shitload of samples
    uint16_t totalSamples = 0;
    uint8_t samples = 0;
    while (samples < NUM_OF_REV_SAMPLES) {
        uint16_t tempREV = adc_measure(0);
        totalSamples++;
        if (tempREV != 0) {
            samplebuffer[samples++] = tempREV;
        }
    }

    us_stopwatch_end();

    // print them out?
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        printf("(%d, %d)\r\n", i, samplebuffer[i]);
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        sum += samplebuffer[i];
    }
    uint16_t average = sum / NUM_OF_REV_SAMPLES;
    printf("totalSamples: %d\r\n", totalSamples);
    printf("average: %d\r\n", average);

    return average;
}

uint16_t get_reverse_sample_test(void) {
    uint16_t samplebuffer[NUM_OF_REV_SAMPLES];
    // clear buffer
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        samplebuffer[i] = 0;
    }

    us_stopwatch_begin();

    // grab a shitload of samples
    uint16_t totalSamples = 0;
    uint8_t samples = 0;
    while (samples < NUM_OF_REV_SAMPLES) {
        uint16_t tempREV = adc_measure(1);
        totalSamples++;
        if (tempREV != 0) {
            samplebuffer[samples++] = tempREV;
        }
    }

    us_stopwatch_end();

    // print them out?
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        printf("(%d, %d)\r\n", i, samplebuffer[i]);
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < NUM_OF_REV_SAMPLES; i++) {
        sum += samplebuffer[i];
    }
    uint16_t average = sum / NUM_OF_REV_SAMPLES;
    printf("totalSamples: %d\r\n", totalSamples);
    printf("average: %d\r\n", average);

    return average;
}

#define NUM_OF_SWR_SAMPLES 16
void SWR_average(void) {
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    // Take our measurements
    for (uint8_t i = 0; i < NUM_OF_SWR_SAMPLES; i++) {
        tempFWD += get_forward_sample();
        tempREV += get_reverse_sample();
    }

    uint8_t bandIndex = decode_frequency_to_band_index(currentRF.frequency);

    // publish the samples and calculate the SWR
    currentRF.forwardADC = (tempFWD / NUM_OF_SWR_SAMPLES);
    currentRF.reverseADC = tempREV;
    currentRF.swrADC =
        (double)currentRF.reverseADC / (double)currentRF.forwardADC;

    currentRF.forwardWatts = RF_sensor_compensation(
        currentRF.forwardADC, &calibrationTable[0][bandIndex]);
    currentRF.reverseWatts = currentRF.forwardWatts * currentRF.swrADC;
    currentRF.swr =
        calculate_SWR_by_watts(currentRF.forwardWatts, currentRF.reverseWatts);
}

// TODO: I don't think this is returning early if SWR is present

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
    // Measure the frequency
    currentRF.frequency = get_frequency();

    // if the Frequency isn't valid then return early
    if (currentRF.frequency == 0xffff) {
        clear_currentRF();
        return -1;
    }

    if (wait_for_stable_FWD() == -1) {
        clear_currentRF();
        return -1;
    }

    SWR_average();

    if (currentRF.forwardADC < 8) {
        clear_currentRF();
    }

    return 0;
}

#define FREQUENCY_SAMPLE_INTERVAL 1000
#define RF_SAMPLE_INTERVAL 100

void RF_sensor_update(void) {
    if (systick_elapsed_time(currentRF.lastFrequencyTime) >
        FREQUENCY_SAMPLE_INTERVAL) {
        LOG_TRACE({ println("updating frequency"); });
        currentRF.lastFrequencyTime = systick_read();
        currentRF.frequency = get_frequency();
    }
    if (systick_elapsed_time(currentRF.lastRFTime) > RF_SAMPLE_INTERVAL) {
        LOG_TRACE({ println("updating RF"); });
        currentRF.lastRFTime = systick_read();
        SWR_average();
    }
    if (currentRF.forwardADC < 8) {
        clear_currentRF();
    }
}