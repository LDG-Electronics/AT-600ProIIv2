#include "includes.h"

/* ************************************************************************** */

// Global RF Readings
RF_power_s currentRF;

// Global SWR Threshold Settings
double swrThresh = SWR1_7;
uint8_t swrThreshIndex = 0;
const double swrThreshTable[] = {SWR1_5, SWR1_7, SWR2_0, SWR2_5, SWR3_0};

/* ************************************************************************** */

void RF_sensor_init(void)
{
    adc_init();
    
    timer3_clock_source(TMR_CLK_FOSC);
    
    // Initialize the Global RF Readings
    currentRF.forward = 0;
    currentRF.reverse = 0;
    currentRF.swr = 0;
    currentRF.period = 0;
    currentRF.frequency = 0;

    // shell commands
    shell_register(shell_get_RF, "getRF");
    
    // Calibration Task
    task_register("swr", task_RF_calibration, 1000, 100);
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

    TODO: write docs
*/

// 
volatile static uint32_t timer3Count;

void __interrupt(irq(TMR3), high_priority) timer3_overflow_counter_ISR(void)
{
    timer3_IF_clear();

    timer3Count += UINT16_MAX;
}

uint16_t count_freq_pin_changes(void)
{
    uint24_t currentTime = systick_read();
    uint16_t freqPinCount = 0;
    uint8_t prevFreqPin = FREQ_PIN;

    while(systick_read() <= (currentTime + 20)){
        if(prevFreqPin != FREQ_PIN){
            prevFreqPin = FREQ_PIN;
            freqPinCount++;
        }
    }

    return freqPinCount;
}

#define PERIOD_THRESHOLD 2
uint32_t get_period(void)
{
    // Make sure there's a frequency signal
    if(count_freq_pin_changes() < PERIOD_THRESHOLD) return 0;

    // Prepare the timer
    timer3_clear();
    timer3_IF_clear();
    timer3Count = 0;
    timer3_interrupt_enable();

    // align ourselves with the rising edge of FREQ_PIN
    while(FREQ_PIN != 0); // while high
    while(FREQ_PIN == 0); // while low

    // engage
    timer3_start();
    while(FREQ_PIN != 0); // while high
    timer3_stop();
    timer3_interrupt_disable();

    // calculate total elapsed time
    timer3Count += timer3_read();
    return timer3Count;
}

/* -------------------------------------------------------------------------- */
/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/
static double calculate_SWR(uint16_t tempFWD, uint16_t tempREV)
{
    double x = sqrt((double)tempREV/(double)tempFWD);
    return ((1.0 + x) / (1.0 - x));
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
    printf("(%u, %u, %f, %lu)\r\n", 
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