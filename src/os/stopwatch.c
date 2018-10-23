#include "stopwatch.h"
#include "../peripherals/timer.h"
#include "system_time.h"

/* ************************************************************************** */

void stopwatch_init(void) {
    // set timer0 to overflow in exactly 1uS
    T0CON0bits.MD16 = 1; // 16 bit mode
    timer0_clock_source(TMR0_CLK_FOSC4);
    timer0_prescale(TMR_PRE_1_16);
}

/* -------------------------------------------------------------------------- */
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

void __interrupt(irq(TMR0), high_priority) us_stopwatch_ISR(void) {
    timer0_IF_clear();

    stopwatchCount += UINT16_MAX;
}

void us_stopwatch_begin(void) {
    // Clear old stuff
    stopwatchCount = 0;
    timer0_IF_clear();
    timer0_clear();

    // Enable interrupt and engage
    timer0_interrupt_enable();
    timer0_start();
}

uint32_t us_stopwatch_end(void) {
    // Disable interrupts and stop
    timer0_stop();
    timer0_interrupt_disable();

    // calculate final value and return
    stopwatchCount += timer0_read();
    return stopwatchCount;
}

/* -------------------------------------------------------------------------- */
/*  Notes on the Millisecond Stopwatch

    This stopwatch does not require a hardware timer. Instead, it uses the
    system tick to count elapsed time between calls to begin() and end().

    Due to the single static variable ms_startTime, this stopwatch breaks when
    multiple pairs of begin() and end() calls overlap.
*/
static system_time_t ms_startTime = 0;

void ms_stopwatch_begin(void) { ms_startTime = systick_read(); }

uint32_t ms_stopwatch_end(void) { return systick_elapsed_time(ms_startTime); }

/* -------------------------------------------------------------------------- */
/*  Notes on the Millisecond Multiwatch

    The multiwatch is a modified ms_stopwatch that supports multiple watches
    "running" simultaneously. No additional hardware resources are needed, only
    minor additional RAM for storing an array of ms_startTime variables.

    The user has to manage thier own stopwatchIDs.
*/
static system_time_t ms_multiwatchStartTimes[CONCURRENT_MULTIWATCHES];

void ms_multiwatch_begin(uint8_t stopwatchID) {
    // stash the start_time
    ms_multiwatchStartTimes[stopwatchID] = systick_read();
}

uint32_t ms_multiwatch_end(uint8_t stopwatchID) {
    return systick_elapsed_time(ms_multiwatchStartTimes[stopwatchID]);
}

/* ************************************************************************** */

#if 0
void stopwatch_and_delay_test(void) {
    println("");
    println("-----------------------------------------------");
    println("microsecond delays with microsecond stopwatch");

    print("10us: ");
    us_stopwatch_begin();
    delay_us(10);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("100us: ");
    us_stopwatch_begin();
    delay_us(100);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("1000us: ");
    us_stopwatch_begin();
    delay_us(1000);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("10000us: ");
    us_stopwatch_begin();
    delay_us(10000);
    printf("%lu uS\r\n", us_stopwatch_end());

    println("");
    println("-----------------------------------------------");
    println("millisecond delays with microsecond stopwatch");
    println("reminder: 1ms = 1000us, 10ms = 10000us");
    println("reminder: delay_ms is expected to have +-1ms jitter");

    print("1ms: ");
    us_stopwatch_begin();
    delay_ms(1);
    printf("%lu uS", us_stopwatch_end());
    println(" <- expected jitter is +-1ms");

    print("10ms: ");
    us_stopwatch_begin();
    delay_ms(10);
    printf("%lu uS", us_stopwatch_end());
    println(" <- expected jitter is +-1ms");

    print("100ms: ");
    us_stopwatch_begin();
    delay_ms(100);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("200ms: ");
    us_stopwatch_begin();
    delay_ms(200);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("500ms: ");
    us_stopwatch_begin();
    delay_ms(500);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("1000ms: ");
    us_stopwatch_begin();
    delay_ms(1000);
    printf("%lu uS\r\n", us_stopwatch_end());

    print("10000ms: ");
    us_stopwatch_begin();
    delay_ms(10000);
    printf("%lu uS\r\n", us_stopwatch_end());

    println("");
    println("-----------------------------------------------");
    println("millisecond delays with millisecond stopwatch");

    print("1ms: ");
    ms_stopwatch_begin();
    delay_ms(1);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("10ms: ");
    ms_stopwatch_begin();
    delay_ms(10);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("100ms: ");
    ms_stopwatch_begin();
    delay_ms(100);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("1000ms: ");
    ms_stopwatch_begin();
    delay_ms(1000);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("2000ms: ");
    ms_stopwatch_begin();
    delay_ms(2000);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("10000ms: ");
    ms_stopwatch_begin();
    delay_ms(10000);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("20000ms: ");
    ms_stopwatch_begin();
    delay_ms(20000);
    printf("%lu mS\r\n", ms_stopwatch_end());

    print("30000ms: ");
    ms_stopwatch_begin();
    delay_ms(30000);
    printf("%lu mS\r\n", ms_stopwatch_end());
}
#endif