#include "includes.h"

/* ************************************************************************** */

// Global RF Readings
RF_power_s currentRF;

// Global SWR Threshold Settings
double swrThresh = SWR1_7;
uint8_t swrThreshIndex = 0;
const double swrThreshTable[5] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};

// file scope 
volatile uint8_t timer3_overflow_count;

/* ************************************************************************** */

void timer3_overflow_counter_init(void)
{
    // timer3
    T3CONbits.RD16 = 1; // Access Timer3 as a single 16 bit operation
    T3CONbits.NOT_SYNC = 1; // Do not synchronize the input with the system clock
    T3CLK = 2; // Clock source is FOSC

    // timer3 gate
    T3GCONbits.GPOL = 1; // Timer3 gate is active-high
    T3GCONbits.GSPM = 1; // Timer3 Gate Single Pulse mode is enabled
    T3GPPS = (PPS_PORT_E & PPS_PIN_0);
}

void RF_sensor_init(void)
{
    adc_init();
    timer3_overflow_counter_init();

    // Initialize the Global RF Readings
    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.swr = 0;
    currentRF.period = 0;
    currentRF.frequency = 0;

    // shell commands
    shell_register(shell_get_RF, "getRF");
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
/*  Notes on the Period counter

    This module uses the timer 3 gate to measure the period of the square wave
    coming in on FREQ_PIN. 

    The timer gate only allows the timer to increment while the gate input is
    active. The gate also has a single pulse mode, only allowing the timer to
    increment for a single pulse of the gate input signal.

    The general procedure for taking a period measurement is as follows:


*/
void __interrupt(irq(TMR3), high_priority) timer3_overflow_counter_ISR(void)
{
    timer3_IF_clear();

    timer3_overflow_count++;
}

uint32_t get_period(void)
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

    // // reset our resources
    // timer3_overflow_count = 0;
    // timer3_stop();
    // timer3_clear();

    // // make sure we don't start the measurement in the middle of a high pulse
    // while(FREQ_PIN == 1);

    // // start the measurement
    // PIE6bits.TMR3IE = 1; // interrupt enable
    // timer3_gate_enable();
    // timer3_start();

    // // Wait for the measurement to finish
    // while(T3GCONbits.GGO == 0);
    // while(T3GCONbits.GGO == 1);

    // // Clean up timer resources
    // timer3_stop();
    // timer3_gate_disable();
    // PIE6bits.TMR3IE = 0; // interrupt disable

    // // If the timer overflowed, calculate the total number of ticks
    // if(timer3_overflow_count) {
    //     uint32_t timer3_total = (timer3_overflow_count * UINT16_MAX);
    //     timer3_total += timer3_read();
    //     return timer3_total;
    // } else {
    //     return timer3_read();
    // }
}

/* -------------------------------------------------------------------------- */
/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/
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
    currentRF.forward = adc_measure(0);
    currentRF.reverse = adc_measure(1);
    currentRF.swr = calculate_SWR(tempFWD, tempREV);  
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
#define NUM_OF_SAMPLES 16
void SWR_average(void)
{
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    // Take our measurements
    for (uint8_t i = 0; i < NUM_OF_SAMPLES; i++)
    {
        tempFWD += adc_measure(0);
        tempREV += adc_measure(1);
    }

    // publish the samples and calculate the SWR
    currentRF.forward = tempFWD;
    currentRF.reverse = tempREV; 
    currentRF.swr = calculate_SWR(tempFWD, tempREV);
}

#define NO_POWER_COUNT 5000
#define LOW_POWER 30

/*  Notes on SWR_stable_average()

    This function monitors the forward power and waits until the slope is flat.

*/
int8_t SWR_stable_average(void)
{
    uint16_t attemptsRemaining = NO_POWER_COUNT;

    int32_t currentFWD;
    int32_t previousFWD = adc_measure(0);

    int32_t deltaFWD = 0;
    int32_t deltaCompare = 0;
    
    while(1)
    {
        currentFWD = adc_measure(0);
        deltaFWD = abs(currentFWD - previousFWD);
        deltaCompare = currentFWD >> 4;
        
        if (currentFWD > LOW_POWER) {
            if (deltaFWD < deltaCompare) break;
        }
        previousFWD = currentFWD;
        
        if (attemptsRemaining == 0) return (-1);
        attemptsRemaining--;
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

/* -------------------------------------------------------------------------- */

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
    printf("(%u, %u, %f, %ul)\r\n", 
            currentRF.forward, currentRF.reverse, currentRF.swr, (uint32_t)currentRF.period);
}

// This task is used to generate calibration tables
void task_RF_calibration(void)
{
    SWR_average();
    currentRF.period = get_period();
    // print_current_time();
    print_RF_calibration_data();
}

/* -------------------------------------------------------------------------- */
// Shell command for RF sensor
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
