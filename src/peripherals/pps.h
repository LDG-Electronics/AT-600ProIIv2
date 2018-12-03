#ifndef _PPS_H_
#define _PPS_H_

#include "device_header.h"

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

    Connecting the Timer 3 Clock input to pin RB0:
    PPS_IN_TIMER3_CLOCK(PPS_PORT_B & PPS_PIN_0);

    Connecting the UART 1 Recieve input to pin RC7:
    PPS_IN_UART1_RX(PPS_PORT_C & PPS_PIN_7);
*/

// Can be used to pass PPS pins into a function
typedef unsigned char pps_input_t;

/* -------------------------------------------------------------------------- */

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

/* -------------------------------------------------------------------------- */

// Interrupt 0  (RB0) | A B - | A B - - - | A B - - - - |
// Interrupt 1  (RB1) | A B - | A B - - - | - B - D - - |
// Interrupt 2  (RB2) | A B - | A B - - - | - B - - - F |

#define PPS_IN_INTERRUPT0(inputPin) INT0PPS = (inputPin)
#define PPS_IN_INTERRUPT1(inputPin) INT1PPS = (inputPin)
#define PPS_IN_INTERRUPT2(inputPin) INT2PPS = (inputPin)

/* -------------------------------------------------------------------------- */

// Timer0 Clock (RA4) | A B - | A B - - - | A - - - - F |
// Timer1 Clock (RC0) | A - C | A - C - - | - - C - E - |
// Timer1 Gate  (RB5) | - B C | - B C - - | - B C - - - |
// Timer3 Clock (RC0) | - B C | - B C - - | - - C - E - |
// Timer3 Gate  (RC0) | A - C | A - C - - | A - C - - - |
// Timer5 Clock (RC2) | A - C | A - C - - | - - C - E - |
// Timer5 Gate  (RB4) | - B C | - B - D - | - B - D - - |
// Timer2 Clock (RC3) | A - C | A - C - - | A - C - - - |
// Timer4 Clock (RC5) | - B C | - B C - - | - B C - - - |
// Timer6 Clock (RB7) | - B C | - B - D - | - B - D - - |

#define PPS_IN_TIMER0_CLOCK(inputPin) T0CKIPPS = (inputPin)
#define PPS_IN_TIMER1_CLOCK(inputPin) T1CKIPPS = (inputPin)
#define PPS_IN_TIMER1_GATE(inputPin) T1GPPS = (inputPin)
#define PPS_IN_TIMER3_CLOCK(inputPin) T3CKIPPS = (inputPin)
#define PPS_IN_TIMER3_GATE(inputPin) T3GPPS = (inputPin)
#define PPS_IN_TIMER5_CLOCK(inputPin) T5CKIPPS = (inputPin)
#define PPS_IN_TIMER5_GATE(inputPin) T5GPPS = (inputPin)
#define PPS_IN_TIMER2_CLOCK(inputPin) T2INPPS = (inputPin)
#define PPS_IN_TIMER4_CLOCK(inputPin) T4INPPS = (inputPin)
#define PPS_IN_TIMER6_CLOCK(inputPin) T6INPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// CCP1 (RC2) | - B C | - B C - - | - - C - - F |
// CCP2 (RC1) | - B C | - B C - - | - - C - - F |
// CCP3 (RB5) | - B C | - B - D - | - B - D - - |
// CCP4 (RB0) | - B C | - B - D - | - B - D - - |

#define PPS_IN_CCP1(inputPin) CCP1PPS = (inputPin)
#define PPS_IN_CCP2(inputPin) CCP2PPS = (inputPin)
#define PPS_IN_CCP3(inputPin) CCP3PPS = (inputPin)
#define PPS_IN_CCP4(inputPin) CCP4PPS = (inputPin)

/* -------------------------------------------------------------------------- */

// SMT1 Window (RC0) | - B C | - B C - - | - - C - - F |
// SMT1 Signal (RC1) | - B C | - B C - - | - - C - - F |

#define PPS_IN_SMT1_WINDOW(inputPin) SMT1WINPPS = (inputPin)
#define PPS_IN_SMT1_SIGNAL(inputPin) SMT1SIGPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// CWG1 (RB0) | - B C | - B - D - | - B - D - - |
// CWG2 (RB1) | - B C | - B - D - | - B - D - - |
// CWG3 (RB2) | - B C | - B - D - | - B - D - - |

#define PPS_IN_CWG1(inputPin) CWG1PPS = (inputPin)
#define PPS_IN_CWG2(inputPin) CWG2PPS = (inputPin)
#define PPS_IN_CWG3(inputPin) CWG3PPS = (inputPin)

/* -------------------------------------------------------------------------- */

// DSM1 Carrier Low     (RA3) | A - C | A - - D - | A - - D - - |
// DSM1 Carrier High    (RA4) | A - C | A - - D - | A - - D - - |
// DSM1 Source          (RA5) | A - C | A - - D - | A - - D - - |

#define PPS_IN_DSM_CARRIER_LOW(inputPin) MD1CARLPPS = (inputPin)
#define PPS_IN_DSM_CARRIER_HIGH(inputPin) MD1CARHPPS = (inputPin)
#define PPS_IN_DSM_SOURCE(inputPin) MD1SRCPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// CLCx Input 1 (RA0) | A - C | A - C - - | A - C - - - |
// CLCx Input 2 (RA1) | A - C | A - C - - | A - C - - - |
// CLCx Input 3 (RB6) | - B C | - B - D - | - B - D - - |
// CLCx Input 4 (RB7) | - B C | - B - D - | - B - D - - |

#define PPS_IN_CLC1_INPUT(inputPin) CLCIN0PPS = (inputPin)
#define PPS_IN_CLC2_INPUT(inputPin) CLCIN1PPS = (inputPin)
#define PPS_IN_CLC3_INPUT(inputPin) CLCIN2PPS = (inputPin)
#define PPS_IN_CLC4_INPUT(inputPin) CLCIN3PPS = (inputPin)

/* -------------------------------------------------------------------------- */

// ADC Conversion Trigger (RB4) | - B C | - B - D - | - B - D - - |

#define PPS_IN_ADC_TRIGGER(inputPin) ADACTPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// SPI1 Clock           (RC3) | - B C | - B C - - | - B C - - - |
// SPI1 Data            (RC4) | - B C | - B C - - | - B C - - - |
// SPI1 Slave Select    (RA5) | A - C | A - - D - | A - - D - - |

#define PPS_IN_SPI1_CLOCK(inputPin) SPI1SCKPPS = (inputPin)
#define PPS_IN_SPI1_DATA_IN(inputPin) SPI1SDIPPS = (inputPin)
#define PPS_IN_SPI1_SS(inputPin) SPI1SSPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// I2C1 Clock   (RC3) | - B C | - B C - - | - B C - - - |
// I2C1 Data    (RC4) | - B C | - B C - - | - B C - - - |
// I2C2 Clock   (RB1) | - B C | - B - D - | - B - D - - |
// I2C2 Data    (RB2) | - B C | - B - D - | - B - D - - |

#define PPS_IN_I2C1_CLOCK(inputPin) I2C1SCLPPS = (inputPin)
#define PPS_IN_I2C1_DATA(inputPin) I2C1SDAPPS = (inputPin)
#define PPS_IN_I2C2_CLOCK(inputPin) I2C2SCLPPS = (inputPin)
#define PPS_IN_I2C2_DATA(inputPin) I2C2SDAPPS = (inputPin)

/* -------------------------------------------------------------------------- */

// UART1 Receive        (RC7) | - B C | - B C - - | - - C - - F |
// UART1 Clear To Send  (RC6) | - B C | - B C - - | - - C - - F |
// UART2 Receive        (RB7) | - B C | - B - D - | - B - D - - |
// UART2 Clear To Send  (RB6) | - B C | - B - D - | - B - D - - |

#define PPS_IN_UART1_RX(inputPin) U1RXPPS = (inputPin)
#define PPS_IN_UART1_CTS(inputPin) U1CTSPPS = (inputPin)
#define PPS_IN_UART2_RX(inputPin) U2RXPPS = (inputPin)
#define PPS_IN_UART2_CTS(inputPin) U2CTSPPS = (inputPin)

/* ************************************************************************** */
/*  PPS Output Registers

    Example PPS Output assignments:

    Assigning UART1 TX to pin RC6:
    PPS_OUT_UART1_TX(RC6PPS)
*/

// Can be used to pass PPS pins into a function
typedef volatile unsigned char pps_output_t;

/* -------------------------------------------------------------------------- */
// ADC Guard Ring Outputs
#define PPS_ADGRDB 0b110010 // | A - C | A - C - - | A - - - - F |
#define PPS_ADGRDA 0b110001 // | A - C | A - C - - | A - - - - F |

#define PPS_OUT_ADC_GUARD_RING_B(outputPin) outputPin = PPS_ADGRDB
#define PPS_OUT_ADC_GUARD_RING_A(outputPin) outputPin = PPS_ADGRDA

/* -------------------------------------------------------------------------- */
// Complementary Waveform Generator 3
#define PPS_CWG3D 0b110000 // | A - C | A - - D - | A - - D - - |
#define PPS_CWG3C 0b101111 // | A - C | A - - D - | A - - D - - |
#define PPS_CWG3B 0b101110 // | A - C | A - - - E | A - - - E - |
#define PPS_CWG3A 0b101101 // | - B C | - B C - - | - B C - - - |

#define PPS_OUT_CWG3D(outputPin) outputPin = PPS_CWG3D
#define PPS_OUT_CWG3C(outputPin) outputPin = PPS_CWG3C
#define PPS_OUT_CWG3B(outputPin) outputPin = PPS_CWG3B
#define PPS_OUT_CWG3A(outputPin) outputPin = PPS_CWG3A

/* -------------------------------------------------------------------------- */
// Complementary Waveform Generator 3
#define PPS_CWG2D 0b101100 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG2C 0b101011 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG2B 0b101010 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG2A 0b101001 // | - B C | - B C - - | - B C - - - |

#define PPS_OUT_CWG2D(outputPin) outputPin = PPS_CWG2D
#define PPS_OUT_CWG2C(outputPin) outputPin = PPS_CWG2C
#define PPS_OUT_CWG2B(outputPin) outputPin = PPS_CWG2B
#define PPS_OUT_CWG2A(outputPin) outputPin = PPS_CWG2A

/* -------------------------------------------------------------------------- */
// Complementary Waveform Generator 3
#define PPS_CWG1D 0b001000 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG1C 0b000111 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG1B 0b000110 // | - B C | - B - D - | - B - D - - |
#define PPS_CWG1A 0b000101 // | - B C | - B C - - | - B C - - - |

#define PPS_OUT_CWG1D(outputPin) outputPin = PPS_CWG1D
#define PPS_OUT_CWG1C(outputPin) outputPin = PPS_CWG1C
#define PPS_OUT_CWG1B(outputPin) outputPin = PPS_CWG1B
#define PPS_OUT_CWG1A(outputPin) outputPin = PPS_CWG1A

/* -------------------------------------------------------------------------- */
// Digital Signal Modulater
#define PPS_DSM1 0b101000 // | A - C | A - - D - | A - - D - - |

#define PPS_OUT_DSM1(outputPin) outputPin = PPS_DSM1

/* -------------------------------------------------------------------------- */
// Reference Clock Output
#define PPS_CLKR 0b100111 // | - B C | - B C - - | - B - - E - |

#define PPS_OUT_CLKR(outputPin) outputPin = PPS_CLKR

/* -------------------------------------------------------------------------- */
// Numerically Controller Oscillator
#define PPS_NCO1 0b100110 // | A - C | A - - D - | A - - D - - |

#define PPS_OUT_NCO1(outputPin) outputPin = PPS_NCO1

/* -------------------------------------------------------------------------- */
// Timer 0 Output
#define PPS_TMR0 0b100101 // | - B C | - B C - - | - - C - - F |

#define PPS_OUT_TIMER0(outputPin) outputPin = PPS_TMR0

/* -------------------------------------------------------------------------- */
// I2C
#define PPS_I2C2_SDA 0b100100 // | - B C | - B - D - | - B - D - - |
#define PPS_I2C2_SCL 0b100011 // | - B C | - B - D - | - B - D - - |
#define PPS_I2C1_SDA 0b100010 // | - B C | - B C - - | - B C - - - |
#define PPS_I2C1_SCL 0b100001 // | - B C | - B C - - | - B C - - - |

#define PPS_OUT_I2C2_DATA(outputPin) outputPin = PPS_I2C2_SDA
#define PPS_OUT_I2C2_CLOCK(outputPin) outputPin = PPS_I2C2_SCL
#define PPS_OUT_I2C1_DATA(outputPin) outputPin = PPS_I2C1_SDA
#define PPS_OUT_I2C1_CLOCK(outputPin) outputPin = PPS_I2C1_SCL

/* -------------------------------------------------------------------------- */
// SPI
#define PPS_SPI1_SS 0b100000  // | A - C | A - - D - | A - - D - - |
#define PPS_SPI1_SDO 0b011111 // | - B C | - B C - - | - B C - - - |
#define PPS_SPI1_SCK 0b011110 // | - B C | - B C - - | - B C - - - |

#define PPS_OUT_SPI1_SS(outputPin) outputPin = PPS_SPI1_SS
#define PPS_OUT_SPI1_DATA_OUT(outputPin) outputPin = PPS_SPI1_SDO
#define PPS_OUT_SPI1_CLOCK(outputPin) outputPin = PPS_SPI1_SCK

/* -------------------------------------------------------------------------- */
// Comparator
#define PPS_C2OUT 0b011101 // | A - C | A - - - E | A - - - E - |
#define PPS_C1OUT 0b011100 // | A - C | A - - D - | A - - D - - |

#define PPS_OUT_COMPARATOR_2(outputPin) outputPin = PPS_C2OUT
#define PPS_OUT_COMPARATOR_1(outputPin) outputPin = PPS_C1OUT

/* -------------------------------------------------------------------------- */
// UART 2
#define PPS_UART2_RTS 0b011000  // | - B C | - B - D - | - B - D - - |
#define PPS_UART2_TXDE 0b010111 // | - B C | - B - D - | - B - D - - |
#define PPS_UART2_TX 0b010110   // | - B C | - B - D - | - B - D - - |

#define PPS_OUT_UART2_RTS(outputPin) outputPin = PPS_UART2_RTS
#define PPS_OUT_UART2_TXDE(outputPin) outputPin = PPS_UART2_TXDE
#define PPS_OUT_UART2_TX(outputPin) outputPin = PPS_UART2_TX

/* -------------------------------------------------------------------------- */
// UART 1
#define PPS_UART1_RTS 0b010101  // | - B C | - B C - - | - - C - - F |
#define PPS_UART1_TXDE 0b010100 // | - B C | - B C - - | - - C - - F |
#define PPS_UART1_TX 0b010011   // | - B C | - B C - - | - - C - - F |

#define PPS_OUT_UART1_RTS(outputPin) outputPin = PPS_UART1_RTS
#define PPS_OUT_UART1_TXDE(outputPin) outputPin = PPS_UART1_TXDE
#define PPS_OUT_UART1_TX(outputPin) outputPin = PPS_UART1_TX
/* -------------------------------------------------------------------------- */
// PWM
#define PPS_PWM8 0b010000 // | A - C | A - - D - | A - - D - - |
#define PPS_PWM7 0b001111 // | A - C | A - C - - | - - C - - F |
#define PPS_PWM6 0b001110 // | A - C | A - - D - | A - - D - - |
#define PPS_PWM5 0b001101 // | A - C | A - C - - | A - - D - F |

#define PPS_OUT_PWM8(outputPin) outputPin = PPS_PWM8
#define PPS_OUT_PWM7(outputPin) outputPin = PPS_PWM7
#define PPS_OUT_PWM6(outputPin) outputPin = PPS_PWM6
#define PPS_OUT_PWM5(outputPin) outputPin = PPS_PWM5

/* -------------------------------------------------------------------------- */
// Capture/Compare
#define PPS_CCP4 0b001100 // | - B C | - B - D - | - B - D - - |
#define PPS_CCP3 0b001011 // | - B C | - B - D - | - B - D - - |
#define PPS_CCP2 0b001010 // | - B C | - B C - - | - - C - - F |
#define PPS_CCP1 0b001001 // | - B C | - B C - - | - - C - - F |

#define PPS_OUT_CCP4(outputPin) outputPin = PPS_CCP4
#define PPS_OUT_CCP3(outputPin) outputPin = PPS_CCP3
#define PPS_OUT_CCP2(outputPin) outputPin = PPS_CCP2
#define PPS_OUT_CCP1(outputPin) outputPin = PPS_CCP1

/* -------------------------------------------------------------------------- */
// Configurable Logic Cell
#define PPS_CLC4OUT 0b000100 // | - B C | - B - D - | - B - D - - |
#define PPS_CLC3OUT 0b000011 // | - B C | - B - D - | - B - D - - |
#define PPS_CLC2OUT 0b000010 // | A - C | A - C - - | A - - - - F |
#define PPS_CLC1OUT 0b000001 // | A - C | A - C - - | A - - - - F |

#define PPS_OUT_CLC4_OUTPUT(outputPin) outputPin = PPS_CLC4OUT
#define PPS_OUT_CLC3_OUTPUT(outputPin) outputPin = PPS_CLC3OUT
#define PPS_OUT_CLC2_OUTPUT(outputPin) outputPin = PPS_CLC2OUT
#define PPS_OUT_CLC1_OUTPUT(outputPin) outputPin = PPS_CLC1OUT

/* -------------------------------------------------------------------------- */
// Default value of all RXYPPS registers at RESET
// Primarily used to revert a PPS configuration
#define PPS_LATxy 0b000000 // | A B C | A B C D E | A B C D E F |

#define PPS_OUT_LATCH(outputPin) outputPin = PPS_LATxy

/* ************************************************************************** */

#endif /* _PPS_H_ */