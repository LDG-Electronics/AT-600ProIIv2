#include "../includes.h"

/* ************************************************************************** */

void systick_init(void)
{
    timer2_clock_source(TMR_CLK_FOSC);
    timer2_prescale(TMR_PRE_1_128);
    timer2_postscale(TMR_POST_1_2);
    timer2_period_set(0xF9);

    timer2_start();

    SMT1CON0bits.STP = 0; // SMT automatically rolls over when it overflows

    SMT1CON1bits.MODE = 0b1000; // Counter mode
    SMT1SIGbits.SSEL = 0b00011; // Signal Selection set to Timer1 output

    SMT1CON0bits.EN = 1; // enable SMT peripheral
    SMT1CON1bits.GO = 1; // Incrementing, acquiring data is enabled

    SMT1TMR = 0;
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
system_time_t systick_read(void)
{
    // Latch the current value of SMT1TMR into SMT1CPR
    SMT1STATbits.CPRUP = 1;

    return SMT1CPR;
}

system_time_t systick_elapsed_time(system_time_t startTime)
{
    return (systick_read() - startTime);
}

void systick_delay(uint16_t mseconds)
{
    system_time_t startTime = systick_read();

    while(systick_elapsed_time(startTime) < mseconds)
    {
        // system_idle_block();
    }
}

/* -------------------------------------------------------------------------- */


void delay_us(uint16_t useconds)
{
    useconds >>= 1;
    while (useconds > 0)
    {
        useconds--;
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
        asm("nop");
        asm("nop");
        asm("nop");
    }
}

//! marked for deprecation
void delay_ms(uint16_t mseconds)
{
    system_time_t startTime = systick_read();

    while(systick_elapsed_time(startTime) < mseconds)
    {
        // empty loop
    }
}