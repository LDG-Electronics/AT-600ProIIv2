#include "includes.h"

/* ************************************************************************** */

// Global RF Readings
RF_power_s currentRF;

// Global SWR Threshold Settings
uint16_t swrThresh = SWR1_7;
uint8_t swrThreshIndex = 0;
const uint16_t swrThreshTable[5] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};


/* ************************************************************************** */

void RF_sensor_init(void)
{    
    timer1_init();
    timer3_init();

    // PPS setup
    T3CKIPPS = 0b00001000;

    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.swr = 0;
    currentRF.frequency = 0;
}

void SWR_threshold_set(void)
{
    swrThresh = swrThreshTable[swrThreshIndex];
}

void SWR_threshold_increment(void)
{
    swrThreshIndex++;
    if (swrThreshIndex == 4) swrThreshIndex = 0;
    SWR_threshold_set();
}

/* -------------------------------------------------------------------------- */

uint16_t get_freq(void)
{
    timer3_clear();

    // TIMER1_IF = 0;
    // TMR1H = 0xE0;
    // TMR1L = 0xC0;

    // TIMER1_ON = 1;
    TIMER3_ON = 1;

    // while ((TIMER1_IF) == 0);
    delay_ms(100);
    
    TIMER3_ON = 0;
    // TIMER1_ON = 0;
    

    return timer3_read();
}

/* -------------------------------------------------------------------------- */

/*  SWR_measure() calculates the SWR from a single sample
    
    This should probably only be used during development and debugging.
    It doesn't make any effort to clean or smooth the results, and therefore
    shouldn't be used to make any real decisions.
*/
void SWR_measure(void)
{
    uint16_t tempFWD = adc_measure(0);
    uint16_t tempREV = adc_measure(1);

    currentRF.forward = tempFWD;
    currentRF.reverse = tempREV; 
    currentRF.swr = (double)tempREV/(double)tempFWD;   
}

/*  SWR_average() calculates the average SWR across several samples
    
    Several alternative techniques were considered, including collecting an
    array of samples, possibly discarding outliers, then calculating an adjusted
    or otherwise filtered mean.
    
    Possible future improvements include fixed-point math or one of several
    techniques including Exponential Moving Average or an Infinite or Finite
    Impulse Response (IIR) filter
*/

#define NUM_OF_SAMPLES 8
#define SAMPLE_FACTOR 3
#define LOW_POWER_THRESHOLD 8

void SWR_average(void)
{
    uint8_t i = 0;
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    #if LOG_LEVEL_RF_SENSOR >= LOG_LABELS
    print_format(BRIGHT, BLUE);
    print_str_ln("\tSWR_average");
    #endif

    for (i = 0; i < NUM_OF_SAMPLES; i++)
    {
        tempFWD += adc_measure(0);
        tempREV += adc_measure(1);
    }
    if (tempFWD > LOW_POWER_THRESHOLD) {
        currentRF.forward = tempFWD;
        currentRF.reverse = tempREV; 
        currentRF.swr = (double)tempREV/(double)tempFWD;
    } else {
        currentRF.forward = 0;
        currentRF.reverse = 0;
        currentRF.swr = 0;
    }
}

#define NO_POWER_COUNT 5000
#define LOW_POWER 30

int8_t SWR_stable_average(void)
{
    uint16_t ADcount = 0;

    int32_t currentFWD;
    int32_t previousFWD;

    int32_t deltaFWD = 0;
    int32_t deltaCompare = 0;

    #if LOG_LEVEL_RF_SENSOR >= LOG_LABELS
    print_format(BRIGHT, BLUE);
    print_str_ln("\tSWR_stable_average");
    #endif
    
    previousFWD = adc_measure(0);
    while(1)
    {
        currentFWD = adc_measure(0);
        deltaFWD = abs(currentFWD - previousFWD);
        deltaCompare = currentFWD >> 4;
        
        if (currentFWD > LOW_POWER) {
            if (deltaFWD < deltaCompare) break;
        }
        if (ADcount > NO_POWER_COUNT) return (-1);
        
        previousFWD = currentFWD;
        ADcount++;
    }
    
    #if LOG_LEVEL_RF_SENSOR >= LOG_ERROR
    if (ADcount != 0) 
    {
        print_format(BRIGHT, CYAN);
        print_cat_ln("\t\tADcount: ", ADcount);
    }
    #endif
    
    SWR_average();
    
    return 0;
}

/* ************************************************************************** */

// tests

/* print_SWR_samples()

    This function takes an SWR sample and prints it if it's different enough 
    from the previous samples.
*/
void print_SWR_samples(uint8_t delta)
{
    uint32_t temp = 0;

    uint16_t prevFWD = currentRF.forward;
    uint16_t prevREV = currentRF.reverse;
    // double prevSWR = currentRF.swr;
    
    uint16_t deltaFWD = 0;
    uint16_t deltaREV = 0;
    // double deltaSWR = 0;

    SWR_average();

    deltaFWD = abs(currentRF.forward - prevFWD);
    deltaREV = abs(currentRF.reverse - prevREV);
    // deltaSWR = fabs(currentRF.swr - prevSWR);

    if ((deltaFWD > delta) || (deltaREV > delta))
    {
        log_current_SWR();
        
        print_ln();
    }
}