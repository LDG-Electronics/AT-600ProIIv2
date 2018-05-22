#include "includes.h"

/* ************************************************************************** */

// Global RF Readings
RF_power_s currentRF;

// Global SWR Threshold Settings
double swrThresh = SWR1_7;
uint8_t swrThreshIndex = 0;
const double swrThreshTable[5] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};

/* ************************************************************************** */

void RF_sensor_init(void)
{
    // Frequency counter
    // T3CONbits.RD16 = 1; // Access Timer3 as a single 16 bit operation
    // T3CONbits.NOT_SYNC = 1; // Do not synchronize the input with the system clock
    T3CONbits.CKPS = 0b10; // Prescale set to 1:4
    T3CLK = 1; // Clock source is FOSC/4
    
    // SWR sensor
    adc_init();

    shell_register(shell_get_RF, "getRF");

    // Initialize the SWR 
    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.swr = 0;
    currentRF.period = 0;
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

uint16_t get_period(void)
{
    timer3_stop();
    timer3_clear();
    timer3_IF_clear();
    
    begin_critical_section();
    timer3_start();
    while (1)
    {
        if (FREQ_PIN != 0) break;
        if (timer3_IF_read() != 0) goto failure;
    }
    timer3_stop();
    timer3_clear();
    
    timer3_start();
    while (1)
    {
        if (FREQ_PIN == 0) break;
        if (timer3_IF_read() != 0) goto failure;
    }
    timer3_stop();
    timer3_clear();
    
    timer3_start();
    while (1)
    {
        if (FREQ_PIN != 0) break;
        if (timer3_IF_read() != 0) goto failure;
    }
    timer3_stop();
    end_critical_section();
    
    return timer3_read();
    
failure:
    timer3_stop();
    end_critical_section();
    
    return 0xffff;
}

/* -------------------------------------------------------------------------- */

double calculate_SWR(uint16_t tempFWD, uint16_t tempREV)
{
    double x = sqrt((double)tempREV/(double)tempFWD);
    return ((1 + x) / (1 - x));
}

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
    currentRF.swr = calculate_SWR(tempFWD, tempREV);  
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
#define LOW_POWER_THRESHOLD 20

void SWR_average(void)
{
    uint8_t i = 0;
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    for (i = 0; i < NUM_OF_SAMPLES; i++)
    {
        tempFWD += adc_measure(0);
        tempREV += adc_measure(1);
    }
    if (tempFWD > LOW_POWER_THRESHOLD) {
        currentRF.forward = tempFWD;
        currentRF.reverse = tempREV; 
        currentRF.swr = calculate_SWR(tempFWD, tempREV);
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
    
    SWR_average();
    
    return 0;
}

/* -------------------------------------------------------------------------- */

void print_current_SWR(void)
{
    printf("FWD: %d, \tREV: %d, \tSWR: %f, P: %d", 
            currentRF.forward, currentRF.reverse, currentRF.swr, currentRF.period);
}

void print_current_SWR_ln(void)
{
    print_current_SWR();

    println("");
}

/*  print_SWR_for_calibration()

    This function provides data to a calibration routine that runs on an LDG
    Servitor. The Servitor uses this data in conjunction with data from a
    Kenwood TS-480 radio and Alpha 4510 wattmeter to generate frequency
    compensation tables to improve the accuracy of the RF sensor.

    (F, R, SWR,      period)
    (0, 0, 0.000000, -1)
*/

void print_RF_calibration_data(void)
{
    printf("(%d, %d, %f, %d)\r\n", 
            currentRF.forward, currentRF.reverse, currentRF.swr, currentRF.period);
}

/* -------------------------------------------------------------------------- */

int shell_get_RF(int argc, char** argv)
{
    if(argc == 1) {
        print_current_SWR_ln();
    } else {
        if(!strcmp(argv[1], "-fwd")) {
            printf("%d\r\n", currentRF.forward);
        } else if(!strcmp(argv[1], "-rev")) {
            printf("%d\r\n", currentRF.reverse);
        } else if(!strcmp(argv[1], "-swr")) {
            printf("%f\r\n", currentRF.swr);
        } else if(!strcmp(argv[1], "-freq")) {
            printf("%d\r\n", currentRF.period);
        } else {
            println("invalid argument");
        }
    }   

    return SHELL_RET_SUCCESS;
}

void task_RF_calibration(void)
{
    SWR_average();
    currentRF.period = get_period();
    // print_current_time();
    print_RF_calibration_data();
}

/* ************************************************************************** */
// tests

/*  print_SWR_samples()

    This function takes an SWR sample and prints it if it's different enough 
    from the previous samples.
*/
void print_SWR_samples(uint8_t delta)
{
    static uint24_t nextUpdateTime = 0;
    uint24_t currentTime = systick_read();

    if(currentTime < nextUpdateTime) return;
    nextUpdateTime = currentTime + 100;

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
        print_current_SWR();
        
        println("");
    }
}
