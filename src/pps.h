#ifndef _PPS_H_
#define	_PPS_H_

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

#define pps_unlock() PPSLOCK = 0x55; \
    PPSLOCK = 0xAA; \
    PPSLOCKbits.PPSLOCKED = 0x00;

#define pps_lock() PPSLOCK = 0x55; \
    PPSLOCK = 0xAA; \
    PPSLOCKbits.PPSLOCKED = 0x01;

/* ************************************************************************** */
/*  PPS Input Registers

    Each peripheral has a PPS register with which the inputs to the peripheral 
    are selected. Inputs include the device pins. Multiple peripherals can 
    operate from the same source simultaneously. Port reads always return the 
    pin level regardless of peripheral PPS selection. If a pin also has analog 
    functions associated, the ANSEL bit for that pin must be cleared to enable 
    the digital input buffer. Although every peripheral has its own PPS input 
    selection register, the selections are identical for every peripheral. 

    Example PPS Input assignments:

    T3CKIPPS = PPS_PORT_B && PPS_PIN_0;
    Connect the Timer 3 Clock input to pin RB0
    
    U1RXPPS = PPS_PORT_C && PPS_PIN_7;
    Connect the UART 1 Recieve input to pin RC7

    Peripheral              Register    Default     Port
                                        Pin         Availability
    -------------------------------------------------------------
    Interrupt 0             INT0PPS     RB0         A B —
    Interrupt 1             INT1PPS     RB1         A B —
    Interrupt 2             INT2PPS     RB2         A B —
    Timer0 Clock            T0CKIPPS    RA4         A B —
    Timer1 Clock            T1CKIPPS    RC0         A — C
    Timer1 Gate             T1GPPS      RB5         — B C
    Timer3 Clock            T3CKIPPS    RC0         — B C
    Timer3 Gate             T3GPPS      RC0         A — C
    Timer5 Clock            T5CKIPPS    RC2         A — C
    Timer5 Gate             T5GPPS      RB4         — B C
    Timer2 Clock            T2INPPS     RC3         A — C
    Timer4 Clock            T4INPPS     RC5         — B C
    Timer6 Clock            T6INPPS     RB7         — B C
    CCP1                    CCP1PPS     RC2         — B C
    CCP2                    CCP2PPS     RC1         — B C
    CCP3                    CCP3PPS     RB5         — B C
    CCP4                    CCP4PPS     RB0         — B C
    SMT1 Window             SMT1WINPPS  RC0         — B C
    SMT1 Signal             SMT1SIGPPS  RC1         — B C
    CWG1                    CWG1PPS     RB0         — B C
    CWG2                    CWG2PPS     RB1         — B C
    CWG3                    CWG3PPS     RB2         — B C
    DSM1 Carrier Low        MD1CARLPPS  RA3         A — C
    DSM1 Carrier High       MD1CARHPPS  RA4         A — C
    DSM1 Source             MD1SRCPPS   RA5         A — C
    CLCx Input 1            CLCIN0PPS   RA0         A — C
    CLCx Input 2            CLCIN1PPS   RA1         A — C
    CLCx Input 3            CLCIN2PPS   RB6         — B C
    CLCx Input 4            CLCIN3PPS   RB7         — B C
    ADC Conversion Trigger  ADACTPPS    RB4         — B C
    SPI1 Clock              SPI1SCKPPS  RC3         — B C
    SPI1 Data               SPI1SDIPPS  RC4         — B C
    SPI1 Slave Select       SPI1SSPPS   RA5         A — C
    I 2 C1 Clock            I2C1SCLPPS  RC3         — B C
    I 2 C1 Data             I2C1SDAPPS  RC4         — B C
    I 2 C2 Clock            I2C2SCLPPS  RB1         — B C
    I 2 C2 Data             I2C2SDAPPS  RB2         — B C
    UART1 Receive           U1RXPPS     RC7         — B C
    UART1 Clear To Send     U1CTSPPS    RC6         — B C
    UART2 Receive           U2RXPPS     RB7         — B C
    UART2 Clear To Send     U2CTSPPS    RB6         — B C
*/

// Input pin assignment macros
#define PPS_PORT_A 0b000111
#define PPS_PORT_B 0b001111
#define PPS_PORT_C 0b010111
#define PPS_PORT_D 0b011111
#define PPS_PORT_E 0b100111
#define PPS_PORT_F 0b101111

#define PPS_PIN_0 0b111000
#define PPS_PIN_1 0b111001
#define PPS_PIN_2 0b111010
#define PPS_PIN_3 0b111011
#define PPS_PIN_4 0b111100
#define PPS_PIN_5 0b111101
#define PPS_PIN_6 0b111110
#define PPS_PIN_7 0b111111

/* ************************************************************************** */
/* PPS Output Registers
    
    Each I/O pin has a PPS register with which the pin output source is 
    selected. With few exceptions, the port TRIS control associated with that 
    pin retains control over the pin output driver. Peripherals that control 
    the pin output driver as part of the peripheral operation will override 
    the TRIS control as needed. These peripherals include: • UART Although 
    every pin has its own PPS peripheral selection register, the selections 
    are identical for every pin

    Example PPS Output assignments:

    RC6PPS = PPS_UART1_TX;
    Connect pin RC6 to the UART 1 Transmit pin
*/

// Output macros
// ADC Guard Ring Outputs
#define PPS_ADGRDB = 0b110010; // A — C
#define PPS_ADGRDA = 0b110001; // A — C

// Complementary Waveform Generator
#define PPS_CWG3D = 0b110000; // A — C
#define PPS_CWG3C = 0b101111; // A — C
#define PPS_CWG3B = 0b101110; // A — C
#define PPS_CWG3A = 0b101101; // — B C
#define PPS_CWG2D = 0b101100; // — B C
#define PPS_CWG2C = 0b101011; // — B C
#define PPS_CWG2B = 0b101010; // — B C
#define PPS_CWG2A = 0b101001; // — B C
#define PPS_CWG1D = 0b001000; // — B C
#define PPS_CWG1C = 0b000111; // — B C
#define PPS_CWG1B = 0b000110; // — B C
#define PPS_CWG1A = 0b000101; // — B C

// Digital Signal Modulater
#define PPS_DSM1 = 0b101000; // A — C

// Reference Clock Output
#define PPS_CLKR = 0b100111; // — B C

// Numerically Controller Oscillator
#define PPS_NCO1 = 0b100110; // A — C

// Timer 0 Output
#define PPS_TMR0 = 0b100101; // — B C

// I2C
#define PPS_I2C2_SDA = 0b100100; // — B C
#define PPS_I2C2_SCL = 0b100011; // — B C
#define PPS_I2C1_SDA = 0b100010; // — B C
#define PPS_I2C1_SCL = 0b100001; // — B C

// SPI
#define PPS_SPI1_SS = 0b100000; // A — C
#define PPS_SPI1_SDO = 0b011111; // — B C
#define PPS_SPI1_SCK = 0b011110; // — B C

// Comparator
#define PPS_C2OUT = 0b011101; // A — C
#define PPS_C1OUT = 0b011100; // A — C

// UART
#define PPS_UART2_RTS = 0b011000; // — B C
#define PPS_UART2_TXDE = 0b010111; // — B C
#define PPS_UART2_TX = 0b010110; // — B C
#define PPS_UART1_RTS = 0b010101; // — B C
#define PPS_UART1_TXDE = 0b010100; // — B C
#define PPS_UART1_TX = 0b010011; // — B C

// PWM
#define PPS_PWM8 = 0b010000; // A — C
#define PPS_PWM7 = 0b001111; // A — C
#define PPS_PWM6 = 0b001110; // A — C
#define PPS_PWM5 = 0b001101; // A — C

// Capture/Compare
#define PPS_CCP4 = 0b001100; // — B C
#define PPS_CCP3 = 0b001011; // — B C
#define PPS_CCP2 = 0b001010; // — B C
#define PPS_CCP1 = 0b001001; // — B C

// Configurable Logic Cell
#define PPS_CLC4OUT = 0b000100; // — B C
#define PPS_CLC3OUT = 0b000011; // — B C
#define PPS_CLC2OUT = 0b000010; // A — C
#define PPS_CLC1OUT = 0b000001; // A — C

#define PPS_LATxy = 0b000000; // A B C


#endif	/* _PPS_H_ */



