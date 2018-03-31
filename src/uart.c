#include "includes.h"
#include "uart.h"

/* ************************************************************************** */

void uart1_init(void)
{
    // PPS Setup
    U1RXPPS = 0b10111; // RX
    RC6PPS = 0b010011; // TX

    U1CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U1CON0bits.TXEN = 1; // Transmit is enabled
    U1CON0bits.MODE = 0b0000; // Asynchronous 8-bit UART mode

    U1BRGL = 0x8A;
    U1BRGH = 0x00;

    U1ERRIR = 0x00; // Clear all error flags

    U1CON1bits.ON = 1; // Enable UART1
}

/* -------------------------------------------------------------------------- */

void uart1_tx_char(const char txData)
{
    while(U1ERRIRbits.TXMTIF == 0);

    U1TXB = txData; // Write the data byte to the USART.
}

// Bitbang a null-terminated string of up to 255 characters
void uart1_tx_string(const char *string)
{
    uint8_t i = 0;

    while (string[i])
    {
        uart1_tx_char(string[i++]);
    }
}


/* ************************************************************************** */
