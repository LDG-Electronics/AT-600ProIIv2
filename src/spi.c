#include "includes.h"
#include "pps.h"
#include "pins.h"

/* ************************************************************************** */

void clc_init(void)
{
    // CLC Setup
    CLC1GLS0 = 0b00000010;
    CLC1GLS1 = 0b00001000;
    CLC1GLS2 = 0b00100000;
    CLC1GLS3 = 0b10000000;

    CLC1SEL0 = 0b101100; // CLC1 data 0, input is SPI Clock
    CLC1SEL1 = 0b101100; // CLC1 data 1, 
    CLC1SEL2 = 0b101100; // CLC1 data 2, 
    CLC1SEL3 = 0b101100; // CLC1 data 3, 
    
    CLC1POL = 0b00000000; // don't invert anything
    
    CLC1CONbits.MODE = 0b010; // 4-input AND mode

    CLC1CONbits.EN = 1; // turn it on

    // CLC Setup
    CLC2GLS0 = 0b00000010;
    CLC2GLS1 = 0b00001000;
    CLC2GLS2 = 0b00100000;
    CLC2GLS3 = 0b10000000;

    CLC2SEL0 = 0b101011; // CLC1 data 0, input is SPI Data
    CLC2SEL1 = 0b101011; // CLC1 data 1, 
    CLC2SEL2 = 0b101011; // CLC1 data 2, 
    CLC2SEL3 = 0b101011; // CLC1 data 3, 
    
    CLC2POL = 0b00000000; // don't invert anything
    
    CLC2CONbits.MODE = 0b010; // 4-input AND mode

    CLC2CONbits.EN = 1; // turn it on
}

void spi_init(void)
{
    clc_init();

    // PPS setup
    RA6PPS = PPS_CLC1OUT; // SCK via CLC1OUT
    RC5PPS = PPS_CLC2OUT; // SDO via CLC2OUT
    // RC4PPS = PPS_SPI1_SS; // SS

    SPI1CON0bits.BMODE = 1;

    SPI1CON1bits.CKE = 0; // Output data changes on transition from idle to active clock state
    SPI1CON1bits.CKP = 1; // Idle state for SCK is high level
    SPI1CON1bits.FST = 0; // Delay to first SCK will be at least ½ baud period
    SPI1CON1bits.SSP = 1; // SS is active-low
    SPI1CON1bits.SDOP = 0; // SDO is active-low

    SPI1CON2bits.SSET = 0; // SS(out) is driven to the active state while the transmit counter is not zero
    SPI1CON2bits.TXR = 1; // TxFIFO data is required for a transfer
    SPI1CON2bits.RXR = 0; // Received data is not stored in the FIFO
    
    SPI1BAUD = 7; // Divide the clock source by 16(FOSC/(2*(BAUD+1)))
    
    SPI1CLK = 0; // FOSC
    
    SPI1CON0bits.MST = 1; // Set as bus master
    SPI1CON0bits.EN = 1; // Enable SPI module
}

/* -------------------------------------------------------------------------- */

void spi_tx_word(uint16_t data)
{
    // Two byte transfer count   
    SPI1TCNTL = 2;
    SPI1TXB = (uint8_t)(data >> 8); // high byte
    SPI1TXB = (uint8_t)(data & 0xff); // low byte 

    while(SPI1STATUSbits.TXBE == 0); // Wait until SPI1TXB is empty

    FP_STROBE_PIN = 0;
    delay_us(10);
    FP_STROBE_PIN = 1;
}

void spi_tx_char(const char data)
{
    while(SPI1STATUSbits.TXBE == 0); // Wait until SPI1TXB is empty

    SPI1TCNTL = 1;
    SPI1TXB = data;
}

void spi_tx_string(const char *string, uint16_t length)
{
    uint16_t i = 0;

    while (i < length)
    {
        spi_tx_char(string[i++]);
    }
}
