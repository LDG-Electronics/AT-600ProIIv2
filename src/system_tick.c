#include "includes.h"

/* ************************************************************************** */

void systick_init(void)
{
    T2CLK = 0b00000010;
    T2CONbits.CKPS = 0b111;
    T2CONbits.OUTPS = 0b0001;
    PR2 = 0xF9;

    timer2_start();

    SMT1CON0bits.STP = 0; // SMT automatically rolls over when it overflows

    SMT1CON1bits.MODE = 0b1000; // Counter mode
    SMT1SIGbits.SSEL = 0b00011; // Signal Selection set to Timer1 output

    SMT1CON0bits.EN = 1; // enable SMT peripheral
    SMT1CON1bits.GO = 1; // Incrementing, acquiring data is enabled

    SMT1TMRL = 0;
    SMT1TMRH = 0;
    SMT1TMRU = 0;
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
uint24_t systick_read(void)
{
    uint24_t result = 0;

    SMT1STATbits.CPRUP = 1; // Latch the current value of SMT1TMR into SMT1CPR

    // Load the latched value into the return variable
    result = (uint24_t)SMT1CPRU << 16; // upper byte
    result += (uint24_t)SMT1CPRH << 8; // high byte
    result += SMT1CPRL; // low byte

    return result;
}