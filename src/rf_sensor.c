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
/*  validate_frequency_signal()

    This function confirms that a valid Frequency signal is present on the input
    of FREQ_PIN. It does this by counting the number of state changes during a
    40 ms window. In traditional UNIX style, return value of 0 means success,
    and -1 means failure.
*/
#define PERIOD_THRESHOLD 2
static int8_t validate_frequency_signal(void)
{
    uint24_t currentTime = systick_read();
    uint16_t freqPinCount = 0;
    uint8_t prevFreqPin = FREQ_PIN;

    while(systick_read() <= (currentTime + 40)){
        if(prevFreqPin != FREQ_PIN){
            prevFreqPin = FREQ_PIN;
            freqPinCount++;
        }
    }

    if(freqPinCount > PERIOD_THRESHOLD) return 0;

    return -1;
}

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

void __interrupt(irq(TMR3), high_priority) timer3_overflow_ISR(void)
{
    timer3_IF_clear();

    timer3Count += UINT16_MAX;
}

uint32_t get_period(void)
{
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

/*  Notes on Frequency Measurement

    Frequency is computed by averaging 4 period measurements and performing a
    big honking integer division. The MAGIC_FREQUENCY_NUMBER is derived from the
    following calculation:
    
    (32,768 / 2) / 15.625 = 1048.576

    1.055 is the magic number to convert observed period to frequency.

    1.055 / 75442 = 1.3984 e-5
    1,055 / 75442 = 0.013984
    1,055,000 / 75442 = 13.984
    1,055,000,000 / 75442 = 13,984 <- ding ding

    Fine adjustments to the end result can be made by adjusting the frequency
    constant. Possible future temperature compensation can be performed by
    adjusting the frequency constant at runtime.
*/

#define MAGIC_FREQUENCY_NUMBER 1057000000
#define NUM_OF_PERIOD_SAMPLES 4
uint16_t get_frequency(void)
{
    // Make sure there's a frequency signal
    if(validate_frequency_signal() == -1) return 0xffff;

    uint32_t tempPeriod = 0;

    // Take measurements
    for (uint8_t i = 0; i < NUM_OF_PERIOD_SAMPLES; i++){
        tempPeriod += get_period();
    }
 
    tempPeriod /= NUM_OF_PERIOD_SAMPLES;

    return (MAGIC_FREQUENCY_NUMBER / tempPeriod);
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
#define NUM_OF_SWR_SAMPLES 16
void SWR_average(void)
{
    uint16_t tempFWD = 0;
    uint16_t tempREV = 0;

    // Take our measurements
    for (uint8_t i = 0; i < NUM_OF_SWR_SAMPLES; i++)
    {
        tempFWD += adc_measure(0);
        tempREV += adc_measure(1);
    }

    // publish the samples and calculate the SWR
    currentRF.forward = (tempFWD / NUM_OF_SWR_SAMPLES);
    currentRF.reverse = (tempREV / NUM_OF_SWR_SAMPLES); 
    currentRF.swr = calculate_SWR(tempFWD, tempREV);
}

/*  Notes on wait_for_stable_FWD()


*/
int8_t wait_for_stable_FWD(void)
{
    uint16_t currentFWD;
    uint16_t previousFWD = adc_measure(0);

    int16_t deltaFWD = 0;
    int16_t deltaCompare = 0;

    // spend up to 500ms waiting for the Forward Power to level off
    uint24_t currentTime = systick_read();
    while(systick_read() <= (currentTime + 500)){
        currentFWD = adc_measure(0);

        deltaFWD = abs((int16_t)currentFWD - (int16_t)previousFWD);
        deltaCompare = currentFWD >> 4;
        if (deltaFWD < deltaCompare) return 0;
        previousFWD = currentFWD;
    }
    return -1;
}

/*  Notes on SWR_stable_average()

    This function monitors the forward power and waits until the slope is flat.


*/
int8_t SWR_stable_average(void)
{
    // Measure the frequency
    currentRF.frequency = get_frequency();

    if(wait_for_stable_FWD() == -1)
    {
        return -1;
    }
    
    SWR_average();
    
    return 0;
}

/* -------------------------------------------------------------------------- */

void print_current_SWR(void)
{
    printf("FWD: %d, \tREV: %d, \tSWR: %f, F: %d", 
            currentRF.forward, currentRF.reverse, currentRF.swr, currentRF.frequency);
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

    (F, R, SWR,      frequency)
    (0, 0, 0.000000, 0xffff)
*/

void print_RF_calibration_data(void)
{
    printf("(%u, %u, %f, %u)\r\n", 
            currentRF.forward, currentRF.reverse, currentRF.swr, currentRF.frequency);
}

// This task is used to generate calibration tables
void task_RF_calibration(void)
{
    SWR_stable_average();

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
            printf("%d\r\n", currentRF.frequency);
        } else {
            println("invalid argument");
        }
    }   

    return SHELL_RET_SUCCESS;
}