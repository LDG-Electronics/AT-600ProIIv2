#include "includes.h"

/* ************************************************************************** */

volatile uint32_t stopwatchCount;

/* ************************************************************************** */

#if LOG_LEVEL_SYSTEM > LOG_SILENT

/* -------------------------------------------------------------------------- */

// TODO: Measure and calibrate this against the revised delay library
void us_stopwatch_begin(void)
{
    T0CON0bits.MD16 = 1; // Timer 0 set to 16 bit mode
    T0CON1bits.CS = 0b010; // clock source set to FOSC/4
    T0CON1bits.CKPS = 0b0100; // prescalar set to 1:8
    
    // Clear old stuff
    stopwatchCount = 0;
    TMR0H = 0;
    TMR0L = 0;

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
    print_cat("stopwatch: ", stopwatchCount);
    print_str_ln("us");
}

// TODO: Measure and calibrate this against the revised delay library
// Include results, especially accuracy measurements. (%)
void ms_stopwatch_begin(void)
{
    T0CON0bits.MD16 = 1; // Timer 0 set to 16 bit mode
    T0CON1bits.CS = 0b010; // clock source set to FOSC/4
    T0CON1bits.CKPS = 0b1001; // prescalar set to 1:256
    
    // Clear old stuff
    stopwatchCount = 0;
    TMR0H = 0;
    TMR0L = 0;

    // Enable interrupt and engage
    PIE3bits.TMR0IE = 1;
    timer0_start();
}

void ms_stopwatch_end(void)
{
    // Disable interrupts and stop
    timer0_stop();
    PIE3bits.TMR0IE = 0;
    
    // Grab final value and print it
    stopwatchCount += timer0_read();
    stopwatchCount /= 30;
    print_cat("stopwatch: ", stopwatchCount);
    print_str_ln("ms");
}

/* -------------------------------------------------------------------------- */

void stopwatch_and_delay_test(void)
{
    // Microsecond tests, microsecond delays
    print_str("10us: ");
    us_stopwatch_begin();
    delay_us(10);
    us_stopwatch_end();
    
    print_str("100us: ");
    us_stopwatch_begin();
    delay_us(100);
    us_stopwatch_end();
    
    print_str("1000us: ");
    us_stopwatch_begin();
    delay_us(1000);
    us_stopwatch_end();
    
    print_str("10000us: ");
    us_stopwatch_begin();
    delay_us(10000);
    us_stopwatch_end();

    print_ln();

    // Microsecond tests, millisecond delays
    print_str("1ms: ");
    us_stopwatch_begin();
    delay_ms(1);
    us_stopwatch_end();
    
    print_str("10ms: ");
    us_stopwatch_begin();
    delay_ms(10);
    us_stopwatch_end();
    
    print_str("100ms: ");
    us_stopwatch_begin();
    delay_ms(100);
    us_stopwatch_end();
    
    print_str("1000ms: ");
    us_stopwatch_begin();
    delay_ms(1000);
    us_stopwatch_end();
    
    print_str("10000ms: ");
    us_stopwatch_begin();
    delay_ms(10000);
    us_stopwatch_end();

    print_ln();
    
    // Millisecond tests
    print_str("1ms: ");
    ms_stopwatch_begin();
    delay_ms(1);
    ms_stopwatch_end();
    
    print_str("10ms: ");
    ms_stopwatch_begin();
    delay_ms(10);
    ms_stopwatch_end();
    
    print_str("100ms: ");
    ms_stopwatch_begin();
    delay_ms(100);
    ms_stopwatch_end();
    
    print_str("1000ms: ");
    ms_stopwatch_begin();
    delay_ms(1000);
    ms_stopwatch_end();
    
    print_str("2000ms: ");
    ms_stopwatch_begin();
    delay_ms(2000);
    ms_stopwatch_end();
    
    print_str("10000ms: ");
    ms_stopwatch_begin();
    delay_ms(10000);
    ms_stopwatch_end();
    
    print_str("20000ms: ");
    ms_stopwatch_begin();
    delay_ms(20000);
    ms_stopwatch_end();
    
    print_str("30000ms: ");
    ms_stopwatch_begin();
    delay_ms(30000);
    ms_stopwatch_end();
}

#endif