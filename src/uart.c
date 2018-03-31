#include "includes.h"
#include "uart.h"
#include "pps.h"

/* ************************************************************************** */

void uart1_init(void)
{
    // PPS Setup
    U1RXPPS = PPS_PORT_C && PPS_PIN_7;
    RC6PPS = PPS_UART1_TX;

    U1CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U1CON0bits.TXEN = 1; // Transmit is enabled
    U1CON0bits.MODE = 0b0000; // Asynchronous 8-bit UART mode

    U1BRGL = 0x8A;
    U1BRGH = 0x00;

    U1ERRIR = 0x00; // Clear all error flags

    U1CON1bits.ON = 1; // Enable UART1
}

/* -------------------------------------------------------------------------- */

void uart1_tx_char(const char data)
{
    while(U1ERRIRbits.TXMTIF == 0);

    U1TXB = data;
}

// Transmit a null-terminated string of up to 255 characters
void uart1_tx_string(const char *string)
{
    uint8_t i = 0;

    while (string[i])
    {
        uart1_tx_char(string[i++]);
    }
}


/* ************************************************************************** */
