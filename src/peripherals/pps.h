#ifndef _PPS_H_
#define _PPS_H_

#include "pic18f46k42.h"

/* ************************************************************************** */

/*  Notes on using the PPS (Peripheral Pin Select)

    The PIC18 K42 family microcontrollers has a Peripheral Pin Select module
    that allows the remapping of digital I/O pins.
*/

/* ************************************************************************** */

/*  Notes on managing the PPS Lock

    These macros are designed to help each software module manage its own PPS
    settings. Writes to PPS registers while the system is locked are ignored.

    Each software module is responsible for putting its required PPS setup code
    in it's module_init(). If all the various module_init() calls are collected
    into one place, like startup(), then the block of _init() calls can be
    wrapped with these two macros, and the entire system's PPS requirements can
    be satisfied in one shot.
*/

#define pps_unlock()                                                           \
    PPSLOCK = 0x55;                                                            \
    PPSLOCK = 0xAA;                                                            \
    PPSLOCKbits.PPSLOCKED = 0x00;

#define pps_lock()                                                             \
    PPSLOCK = 0x55;                                                            \
    PPSLOCK = 0xAA;                                                            \
    PPSLOCKbits.PPSLOCKED = 0x01;

/* ************************************************************************** */
/*  PPS Input Registers

    Example PPS Input assignments:

    T3CKIPPS = (PPS_PORT_B & PPS_PIN_0);
    Connect the Timer 3 Clock input to pin RB0

    U1RXPPS = (PPS_PORT_C & PPS_PIN_7);
    Connect the UART 1 Recieve input to pin RC7

    TODO: Write notes here in native english - not Microchipese

    Peripheral              Register    Default  Port Availability
                                        Pin      2XK42   4XK42       5XK42
    -------------------------------------------------------------
    Interrupt 0             INT0PPS     RB0      A B — | A B — — — | A B — — — —
    Interrupt 1             INT1PPS     RB1      A B — | A B — — — | — B — D — —
    Interrupt 2             INT2PPS     RB2      A B — | A B — — — | — B — — — F
    Timer0 Clock            T0CKIPPS    RA4      A B — | A B — — — | A — — — — F
    Timer1 Clock            T1CKIPPS    RC0      A — C | A — C — — | — — C — E —
    Timer1 Gate             T1GPPS      RB5      — B C | — B C — — | — B C — — —
    Timer3 Clock            T3CKIPPS    RC0      — B C | — B C — — | - — C — E —
    Timer3 Gate             T3GPPS      RC0      A — C | A — C — — | A — C — — —
    Timer5 Clock            T5CKIPPS    RC2      A — C | A — C — — | — — C — E —
    Timer5 Gate             T5GPPS      RB4      — B C | — B — D — | — B — D — —
    Timer2 Clock            T2INPPS     RC3      A — C | A — C — — | A — C — — —
    Timer4 Clock            T4INPPS     RC5      — B C | — B C — — | — B C — — —
    Timer6 Clock            T6INPPS     RB7      — B C | — B — D — | — B — D — —
    CCP1                    CCP1PPS     RC2      — B C | — B C — — | — — C — — F
    CCP2                    CCP2PPS     RC1      — B C | — B C — — | — — C — — F
    CCP3                    CCP3PPS     RB5      — B C | — B — D — | — B — D — —
    CCP4                    CCP4PPS     RB0      — B C | — B — D — | — B — D — —
    SMT1 Window             SMT1WINPPS  RC0      — B C | — B C — — | — — C — — F
    SMT1 Signal             SMT1SIGPPS  RC1      — B C | — B C — — | — — C — — F
    CWG1                    CWG1PPS     RB0      — B C | — B — D — | — B — D — —
    CWG2                    CWG2PPS     RB1      — B C | — B — D — | — B — D — —
    CWG3                    CWG3PPS     RB2      — B C | — B — D — | — B — D — —
    DSM1 Carrier Low        MD1CARLPPS  RA3      A — C | A — — D — | A — — D — —
    DSM1 Carrier High       MD1CARHPPS  RA4      A — C | A — — D — | A — — D — —
    DSM1 Source             MD1SRCPPS   RA5      A — C | A — — D — | A — — D — —
    CLCx Input 1            CLCIN0PPS   RA0      A — C | A — C — — | A — C — — —
    CLCx Input 2            CLCIN1PPS   RA1      A — C | A — C — — | A — C — — —
    CLCx Input 3            CLCIN2PPS   RB6      — B C | — B — D — | — B — D — —
    CLCx Input 4            CLCIN3PPS   RB7      — B C | — B — D — | — B — D — —
    ADC Conversion Trigger  ADACTPPS    RB4      — B C | — B — D — | — B — D — —
    SPI1 Clock              SPI1SCKPPS  RC3      — B C | — B C — — | — B C — — —
    SPI1 Data               SPI1SDIPPS  RC4      — B C | — B C — — | — B C — — —
    SPI1 Slave Select       SPI1SSPPS   RA5      A — C | A — — D — | A — — D — —
    I 2 C1 Clock            I2C1SCLPPS  RC3      — B C | — B C — — | — B C — — —
    I 2 C1 Data             I2C1SDAPPS  RC4      — B C | — B C — — | — B C — — —
    I 2 C2 Clock            I2C2SCLPPS  RB1      — B C | — B — D — | — B — D — —
    I 2 C2 Data             I2C2SDAPPS  RB2      — B C | — B — D — | — B — D — —
    UART1 Receive           U1RXPPS     RC7      — B C | — B C — — | — — C — — F
    UART1 Clear To Send     U1CTSPPS    RC6      — B C | — B C — — | — — C — — F
    UART2 Receive           U2RXPPS     RB7      — B C | — B — D — | — B — D — —
    UART2 Clear To Send     U2CTSPPS    RB6      — B C | — B — D — | — B — D — —
*/

// Input pin assignment macros
#define PPS_PORT_A 0b11000111
#define PPS_PORT_B 0b11001111
#define PPS_PORT_C 0b11010111
#define PPS_PORT_D 0b11011111
#define PPS_PORT_E 0b11100111
#define PPS_PORT_F 0b11101111

#define PPS_PIN_0 0b11111000
#define PPS_PIN_1 0b11111001
#define PPS_PIN_2 0b11111010
#define PPS_PIN_3 0b11111011
#define PPS_PIN_4 0b11111100
#define PPS_PIN_5 0b11111101
#define PPS_PIN_6 0b11111110
#define PPS_PIN_7 0b11111111

/* ************************************************************************** */
/* PPS Output Registers

    Example PPS Output assignments:

    RC6PPS = PPS_UART1_TX;
    Connect pin RC6 to the UART 1 Transmit pin

    TODO: Write notes here in native english - not Microchipese
*/

// Output macros
// ADC Guard Ring Outputs
#define PPS_ADGRDB 0b110010
#define PPS_ADGRDA 0b110001

// Complementary Waveform Generator
#define PPS_CWG3D 0b110000
#define PPS_CWG3C 0b101111
#define PPS_CWG3B 0b101110
#define PPS_CWG3A 0b101101
#define PPS_CWG2D 0b101100
#define PPS_CWG2C 0b101011
#define PPS_CWG2B 0b101010
#define PPS_CWG2A 0b101001
#define PPS_CWG1D 0b001000
#define PPS_CWG1C 0b000111
#define PPS_CWG1B 0b000110
#define PPS_CWG1A 0b000101

// Digital Signal Modulater
#define PPS_DSM1 0b101000

// Reference Clock Output
#define PPS_CLKR 0b100111

// Numerically Controller Oscillator
#define PPS_NCO1 0b100110

// Timer 0 Output
#define PPS_TMR0 0b100101

// I2C
#define PPS_I2C2_SDA 0b100100
#define PPS_I2C2_SCL 0b100011
#define PPS_I2C1_SDA 0b100010
#define PPS_I2C1_SCL 0b100001

// SPI
#define PPS_SPI1_SS 0b100000
#define PPS_SPI1_SDO 0b011111
#define PPS_SPI1_SCK 0b011110

// Comparator
#define PPS_C2OUT 0b011101
#define PPS_C1OUT 0b011100

// UART
#define PPS_UART2_RTS 0b011000
#define PPS_UART2_TXDE 0b010111
#define PPS_UART2_TX 0b010110
#define PPS_UART1_RTS 0b010101
#define PPS_UART1_TXDE 0b010100
#define PPS_UART1_TX 0b010011

// PWM
#define PPS_PWM8 0b010000
#define PPS_PWM7 0b001111
#define PPS_PWM6 0b001110
#define PPS_PWM5 0b001101

// Capture/Compare
#define PPS_CCP4 0b001100
#define PPS_CCP3 0b001011
#define PPS_CCP2 0b001010
#define PPS_CCP1 0b001001

// Configurable Logic Cell
#define PPS_CLC4OUT 0b000100
#define PPS_CLC3OUT 0b000011
#define PPS_CLC2OUT 0b000010
#define PPS_CLC1OUT 0b000001

// Default value of all RXYPPS registers at RESET
// Primarily used to revert a PPS configuration
#define PPS_LATxy 0b000000

#endif /* _PPS_H_ */