#include "system_time.h"
#include "../peripherals/timer.h"

/* ************************************************************************** */

void system_time_init(void) {
    // set timer2 to overflow in exactly 1mS
    timer2_clock_source(TMR_CLK_FOSC);
    timer2_prescale(TMR_PRE_1_128);
    timer2_postscale(TMR_POST_1_2);
    timer2_period_set(0xF9);

    timer2_start();

    // set Signal Measurement Timer to count the number of timer2 overflows
    SMT1CON0bits.STP = 0; // SMT automatically rolls over when it overflows
    SMT1CON1bits.MODE = 0b1000; // Counter mode
    SMT1SIGbits.SSEL = 0b00011; // Signal Selection set to Timer2 output
    SMT1TMR = 0;

    SMT1CON0bits.EN = 1; // enable SMT peripheral
    SMT1CON1bits.GO = 1; // Incrementing, acquiring data is enabled
}

/* -------------------------------------------------------------------------- */

/*  Notes on reading the Signal Measurement Timer

    The SMT is a 24 bit counter, whose value is stored in three registers:
    SMT1TMRL
    SMT1TMRH
    SMT1TMRU

    This means that reading the value of the SMT is very far from an atomic
    operation. The processor provides a tool to solve this problem in the form
    of three SMT Captured Period Registers:
    SMT1CPRL
    SMT1CPRH
    SMT1CPRU

    When SMT1STATbits.CPRUP is set, the value of SMT1TMR is snapshotted into
    SMT1CPR, and therefore is safe to read without worrying about corrupted
    data.
*/
system_time_t get_current_time(void) {
    // Latch the current value of SMT1TMR into SMT1CPR
    SMT1STATbits.CPRUP = 1;

    return SMT1CPR;
}

/* -------------------------------------------------------------------------- */

void delay_us(uint16_t microSeconds) {
    while (microSeconds--) {
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

void delay_ms(uint16_t milliSeconds) {
    system_time_t startTime = get_current_time();

    while (time_since(startTime) < milliSeconds) {
        // empty loop
    }
}