#include "rf_sensor.h"
#include "calibration.h"
#include "os/log_macros.h"
#include "os/shell/shell_json.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "peripherals/adc.h"
#include "peripherals/pins.h"
#include "peripherals/timer.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// Global RF Readings
RF_power_t currentRF;

// json object that matches
const json_field_t JSONcurrentRF[] = {
    {"forward", &(currentRF.forward), jsonFloat},
    {"reverse", &(currentRF.reverse), jsonFloat},
    {"matchQuality", &(currentRF.matchQuality), jsonFloat},
    {"forwardWatts", &(currentRF.forwardWatts), jsonFloat},
    {"reverseWatts", &(currentRF.reverseWatts), jsonFloat},
    {"swr", &(currentRF.swr), jsonFloat},
    {"frequency", &(currentRF.frequency), jsonU16},
    {NULL, NULL, jsonObject},
};

void print_RF_data(void) {
    //
    json_serialize_and_print(&JSONcurrentRF[0]);
}

/* ************************************************************************** */

static void clear_currentRF(void) {
    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.matchQuality = 0.0;
    currentRF.forwardWatts = 0.0;
    currentRF.reverseWatts = 0.0;
    currentRF.swr = 0.0;
    currentRF.frequency = 0xffff;
}

/* -------------------------------------------------------------------------- */

void RF_sensor_init(void) {
    adc_init();

    // Initialize the Global RF Readings
    clear_currentRF();
    currentRF.lastFrequencyTime = 0;
    currentRF.lastRFTime = 0;

    // Frequency counter uses timer3 and timer4
    // timer3 measures the period length
    timer3_clock_source(TMR_CLK_FOSC);

    // timeout timer: overflows in 16.384 mS, according MPLABX
    timer4_clock_source(TMR_CLK_FOSC4);
    timer4_prescale(TMR_PRE_1_128);
    timer4_postscale(TMR_POST_1_8);
    timer4_period_set(0xFF);

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
        sum += adc_read(0);
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

#define NUM_OF_SWR_SAMPLES 32
void measure_RF(void) {
    currentRF.lastRFTime = get_current_time();
    float tempForward = 0;
    float tempReverse = 0;

    // Collect measurements
    for (uint8_t i = 0; i < NUM_OF_SWR_SAMPLES; i++) {
        tempForward += adc_read(0);
        tempReverse += adc_read(1);
    }

    currentRF.forward = tempForward / NUM_OF_SWR_SAMPLES;
    currentRF.reverse = tempReverse / NUM_OF_SWR_SAMPLES;

    currentRF.matchQuality = currentRF.reverse / currentRF.forward;

    // printf("(%lu, %lu)\r\n", tempFWD, tempREV);

    uint8_t bandIndex = decode_frequency_to_band_index(currentRF.frequency);
    currentRF.forwardWatts = RF_sensor_compensation(
        currentRF.forward, &calibrationTable[0][bandIndex]);
    currentRF.reverseWatts = RF_sensor_compensation(
        currentRF.reverse, &calibrationTable[1][bandIndex]);
    currentRF.swr =
        calculate_SWR_by_watts(currentRF.forwardWatts, currentRF.reverseWatts);
}

/* ************************************************************************** */

/*  Notes on period measurement

    Due to hardware decisions that have already been made, measuring Frequency
    directly is impossible. All current LDG products use a frequency counter
    design that divides the incoming signal by 32,768. This is SO SLOW that it
    would require more than a full second to make a useful measurement. This
    issue is compounded by the fact that many products place the frequency
    counter input on Port E. The PIC18F2XK42 cannot remap any timer clock source
    OR configurable logic cell input pins onto Port E, meaning that none of the
    process can be offloaded to hardware.

    This period counter is a (highly) refined version of the one designed by
    Russ Hoffman. It sets up a hardware timer, starts it from software at the
    rising edge of the incoming signal, and stops it at the falling edge. The
    timer is configured to run as fast as possible. An early design concern was
    how to best calibrate the timer to both maximize accuracy AND maintain
    fidelity across the entire range of inputs (1-50 MHz). One possible solution
    was a two-stage measurement, using a preliminary, less accurate period
    measurement followed by a more precise and specific measurement. This idea
    was rejected in favor of the current solution: Just measure a big freaking
    number.

    This brings us back to the timer overflow. Since there is no "best" timer
    configuration that preserves both range of input AND accuracy in a 16 bit
    result, the chosen solution to this problem is to run the timer as fast as
    possible and use timer3_overflow_ISR() to count through the overflow.

    The timer clock source is set to the raw FOSC, the 64 MHz internal
    oscillator. At this speed, 1 timer tick = 15.625 nanoseconds.

    >>> Trigger warning: Math ahead <<<

    A 14 MHz square wave has a period of 71.428 nanoseconds.

    The incoming signal is 32k times slower than the original, therefore each
    square wave is 32k times longer.
    71.428 * 32,768 = 2,340,552

    But wait, we're only measuring a half period.
    2,340,552 / 2 = 1,170,276

    And we're measuring in increments of 15.625 nanoseconds.
    1,170,276 / 15.625 = 74,897

    Therefore, the expected period measurement is in the ballpark of 75,000,
    while the observed period measurement is 75,442.

    That's a margin of error of 0.7%.

    Damn.

    Frequency       Period      Period
                    (expected)  (observed)
    F: 01.800 MHz   Pe: 555ns   Po: 586271
    F: 03.500 MHz   Pe: 285ns   Po: 301675
    F: 07.000 MHz   Pe: 142ns   Po: 150797
    F: 10.100 MHz   Pe: 99ns    Po: 104522
    F: 14.000 MHz   Pe: 71ns    Po: 075442
    F: 18.068 MHz   Pe: 55ns    Po: 058451
    F: 21.000 MHz   Pe: 47ns    Po: 050267
    F: 24.890 MHz   Pe: 40ns    Po: 042431
    F: 28.000 MHz   Pe: 35ns    Po: 037715
    F: 50.000 MHz   Pe: 20ns    Po: 021131
*/

volatile static uint32_t timer3Count;

void __interrupt(irq(TMR3), high_priority) timer3_overflow_ISR(void) {
    timer3_IF_clear();

    timer3Count += UINT16_MAX;
}

uint32_t get_period(void) {
    LOG_TRACE({ println("get_period"); });
    // counting timer
    timer3_clear();
    timer3_IF_clear();
    timer3Count = 0;
    timer3_interrupt_enable();

    // timeout timer
    timer4_clear();
    timer4_IF_clear();
    timer4_start();

    // align ourselves with the falling edge of FREQ_PIN
    while (FREQ_PIN == 0) {
        if (timer4_IF_read()) {
            timer4_stop();
            LOG_ERROR({ println("timed out"); });
            return 0;
        }
    }

    timer4_stop();
    timer4_clear();
    timer4_IF_clear();
    timer4_start();

    // this time it's for real
    timer3_start();
    while (FREQ_PIN != 0) {
        if (timer4_IF_read()) {
            timer4_stop();
            LOG_ERROR({ println("timed out"); });
            return 0;
        }
    }
    timer3_stop();
    timer3_interrupt_disable();
    timer4_stop();

    // calculate total elapsed time
    timer3Count += timer3_read();
    LOG_DEBUG({ printf("period: %lu\r\n", timer3Count); });
    return timer3Count;
}

/*  Notes on Frequency Measurement

    Frequency is computed by averaging 4 period measurements and performing a
    big honking integer division. The MAGIC_FREQUENCY_NUMBER is derived from the
    following calculation:

    The incoming signal is divided by 32,768
    Divide this by 2 because we're measuring a half-cycle
    15.625 is the lenth of a clock cycle, in nanoSeconds

    (32,768 / 2) / 15.625 = 1048.576

    ~1050 is the magic number to convert observed period to frequency.

    The measured period at 14MHz is ~75442

    1,055 / 75442 = 0.013984 <- hey, that's almost a 14
    1,055,000 / 75442 = 13.984
    1,055,000,000 / 75442 = 13,984 <- ding ding

    Fine adjustments to the end result can be made by adjusting the frequency
    constant. Possible future temperature compensation can be performed by
    adjusting the frequency constant at runtime.
*/

#define MAGIC_FREQUENCY_NUMBER 1057000000
#define NUM_OF_PERIOD_SAMPLES 4
void measure_frequency(void) {
    LOG_TRACE({ println("measure_frequency"); });
    currentRF.lastFrequencyTime = get_current_time();
    uint32_t result = 0;
    uint32_t tempPeriod = 0;

    // collect period measurements
    for (uint8_t i = 0; i < NUM_OF_PERIOD_SAMPLES; i++) {
        result = get_period();
        if (result == 0) {
            currentRF.frequency = UINT16_MAX;
            return;
        }
        tempPeriod += result;
    }

    tempPeriod /= NUM_OF_PERIOD_SAMPLES;

    currentRF.frequency = (uint16_t)(MAGIC_FREQUENCY_NUMBER / tempPeriod);

    LOG_INFO({ printf("frequency: %u\r\n", currentRF.frequency); });
}