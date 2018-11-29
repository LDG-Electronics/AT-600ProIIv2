#ifndef _CLC_H_
#define _CLC_H_

#include <stdint.h>

/* ************************************************************************** */
/*  CLC Data Input Selection

    These macros enumerate the possible selections for the CLC's Data Input,
    controlled by CLCxSEL0 through CLCxSEL3.
*/

#define CLC_CWG3B_OUT 110011
#define CLC_CWG3A_OUT 110010
#define CLC_CWG2B_OUT 110001
#define CLC_CWG2A_OUT 110000
#define CLC_CWG1B_OUT 101111
#define CLC_CWG1A_OUT 101110
#define CLC_SS1 101101
#define CLC_SCK1 101100
#define CLC_SDO1 101011
#define CLC_UART2_TX_OUT 101001
#define CLC_UART1_TX_OUT 101000
#define CLC_CLC4_OUT 100111
#define CLC_CLC3_OUT 100110
#define CLC_CLC2_OUT 100101
#define CLC_CLC1_OUT 100100
#define CLC_DSM1_OUT 100011
#define CLC_IOC_FLAG 100010
#define CLC_ZCD_OUT 100001
#define CLC_CMP2_OUT 100000
#define CLC_CMP1_OUT 011111
#define CLC_NCO1_OUT 011110
#define CLC_PWM8_OUT 011011
#define CLC_PWM7_OUT 011010
#define CLC_PWM6_OUT 011001
#define CLC_PWM5_OUT 011000
#define CLC_CCP4_OUT 010111
#define CLC_CCP3_OUT 010110
#define CLC_CCP2_OUT 010101
#define CLC_CCP1_OUT 010100
#define CLC_SMT1_OUT 010011
#define CLC_TMR6_OUT 010010
#define CLC_TMR5_OVERFLOW 010001
#define CLC_TMR4_OUT 010000
#define CLC_TMR3_OVERFLOW 001111
#define CLC_TMR2_OUT 001110
#define CLC_TMR1_OVERFLOW 001101
#define CLC_TMR0_OVERFLOW 001100
#define CLC_CLKR_OUT 001011
#define CLC_ADCRC 001010
#define CLC_SOSC 001001
#define CLC_MFINTOSC_32 001000
#define CLC_MFINTOSC_500 000111
#define CLC_LFINTOSC 000110
#define CLC_HFINTOSC 000101
#define CLC_FOSC 000100
#define CLC_CLCIN3PPS 000011
#define CLC_CLCIN2PPS 000010
#define CLC_CLCIN1PPS 000001
#define CLC_CLCIN0PPS 000000

/* ************************************************************************** */

extern void clc1_passthrough_init(uint8_t dataInput);
extern void clc2_passthrough_init(uint8_t dataInput);
extern void clc3_passthrough_init(uint8_t dataInput);
extern void clc4_passthrough_init(uint8_t dataInput);

/* ************************************************************************** */

#endif