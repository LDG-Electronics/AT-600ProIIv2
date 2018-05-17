#include "includes.h"

/* ************************************************************************** */

#if LOG_LEVEL_SYSTEM > LOG_SILENT

/* ************************************************************************** */

void stopwatch_init(void)
{
    T0CON0bits.MD16 = 1; // Timer 0 set to 16 bit mode
    T0CON1bits.CS = 0b010; // clock source set to FOSC/4(16MHz)
    T0CON1bits.CKPS = 0b0100; // prescaler set to 1:16
}

/*  Notes on us_stopwatch_ISR() and the Microsecond Stopwatch

    This function is an Interrupt Vector Table compatible ISR to respond to the
    TMR0 interrupt signal. This signal is generated whenever timer0 overflows
    from 0xffff to 0x0000. The timer DOES automatically overflow, and therefore
    does not need to be reset from in side the ISR. The Timer 0 Interrupt Flag
    does need be cleared by software.

    The purpose of this interrupt is to allow the microsecond stopwatch to count
    times that are too large to fit in the 16 bits of timer0. When timer0
    overflows(hits 0xffff and resets to 0x0000), us_stopwatch_ISR() will add
    0xffff to stopwatchCount, acting as a pseudo extension to the timer0
    counter. When us_stopwatch_end() is called, timer0 is added to this
    accumulated total and printed to the debug console.

    Timer0 is configured in 16 bit mode, its clock source is set to FOSC/4, or
    16MHz, and its prescaler is set to 1:16. It increments by 1 every 16 clock
    cycles, or at 1MHz. 1MHz = 1,000,000Hz, each timer tick is therefore 
    1/1,000,000th of a second, or exactly 1uS.
*/

volatile static uint32_t stopwatchCount;

void __interrupt(irq(TMR0), high_priority) us_stopwatch_ISR(void)
{
    timer0_IF_clear();

    stopwatchCount += UINT16_MAX;
}

// TODO: Measure and calibrate this against the revised delay library
void us_stopwatch_begin(void)
{
    // Clear old stuff
    stopwatchCount = 0;
    timer0_clear();

    // Enable interrupt and engage
    PIE3bits.TMR0IE = 1;
    timer0_start();
}

void us_stopwatch_end(void)
{
    // Disable interrupts and stop
    timer0_stop();
    PIE3bits.TMR0IE = 0;
    
    // Grab final value and print it
    stopwatchCount += timer0_read();
    printf("stopwatch: %dus\r\n", stopwatchCount);
}

static uint24_t ms_startTime = 0;

void ms_stopwatch_begin(void)
{
    ms_startTime = systick_read();
}

void ms_stopwatch_end(void)
{
    uint24_t currentTime = systick_read();

    printf("stopwatch: %dus\r\n", stopwatchCount);
}

/* -------------------------------------------------------------------------- */

void stopwatch_and_delay_test(void)
{
    // Microsecond tests, microsecond delays
    print("10us: ");
    us_stopwatch_begin();
    delay_us(10);
    us_stopwatch_end();
    
    print("100us: ");
    us_stopwatch_begin();
    delay_us(100);
    us_stopwatch_end();
    
    print("1000us: ");
    us_stopwatch_begin();
    delay_us(1000);
    us_stopwatch_end();
    
    print("10000us: ");
    us_stopwatch_begin();
    delay_us(10000);
    us_stopwatch_end();

    println("");

    // Microsecond tests, millisecond delays
    print("1ms: ");
    us_stopwatch_begin();
    delay_ms(1);
    us_stopwatch_end();
    
    print("10ms: ");
    us_stopwatch_begin();
    delay_ms(10);
    us_stopwatch_end();
    
    print("100ms: ");
    us_stopwatch_begin();
    delay_ms(100);
    us_stopwatch_end();
    
    print("1000ms: ");
    us_stopwatch_begin();
    delay_ms(1000);
    us_stopwatch_end();
    
    print("10000ms: ");
    us_stopwatch_begin();
    delay_ms(10000);
    us_stopwatch_end();

    println("");
    
    // Millisecond tests
    print("1ms: ");
    ms_stopwatch_begin();
    delay_ms(1);
    ms_stopwatch_end();
    
    print("10ms: ");
    ms_stopwatch_begin();
    delay_ms(10);
    ms_stopwatch_end();
    
    print("100ms: ");
    ms_stopwatch_begin();
    delay_ms(100);
    ms_stopwatch_end();
    
    print("1000ms: ");
    ms_stopwatch_begin();
    delay_ms(1000);
    ms_stopwatch_end();
    
    print("2000ms: ");
    ms_stopwatch_begin();
    delay_ms(2000);
    ms_stopwatch_end();
    
    print("10000ms: ");
    ms_stopwatch_begin();
    delay_ms(10000);
    ms_stopwatch_end();
    
    print("20000ms: ");
    ms_stopwatch_begin();
    delay_ms(20000);
    ms_stopwatch_end();
    
    print("30000ms: ");
    ms_stopwatch_begin();
    delay_ms(30000);
    ms_stopwatch_end();
}

#endif