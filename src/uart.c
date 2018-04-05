#include "includes.h"
#include "uart.h"
#include "pps.h"
#include "xc.h"

/* ************************************************************************** */
// Circular buffer tools
#define BUFFER_SIZE 256

typedef struct{   
    uint8_t contents[BUFFER_SIZE];
    uint8_t newest_index;
    uint8_t oldest_index;
}uart_buffer_s;

uart_buffer_s uart1_tx_buffer;

/* ************************************************************************** */

void uart1_init(void)
{
    // PPS Setup
    U1RXPPS = (PPS_PORT_C || PPS_PIN_7);
    RC6PPS = PPS_UART1_TX;

    U1CON0bits.BRGS = 1; // Baud Rate is set to high speed
    U1CON0bits.TXEN = 1; // Transmit is enabled
    U1CON0bits.MODE = 0b0000; // Asynchronous 8-bit UART mode

    U1BRGL = 0x8A;
    U1BRGH = 0x00;

    U1ERRIR = 0x00; // Clear all error flags

    uart1_tx_buffer.newest_index = 0;
    uart1_tx_buffer.oldest_index = 0;

    U1CON1bits.ON = 1; // Enable UART1
}

void __interrupt(irq(IRQ_U1TX), high_priority) U1TX_ISR()
{
    PIR3bits.U1TXIF = 0;
    if(uart1_tx_buffer.newest_index == uart1_tx_buffer.oldest_index)
    {
        PIE3bits.U1TXIE = 0; // buffer is empty now, disable the interrupt
        return;
    }

    U1TXB = uart1_tx_buffer.contents[uart1_tx_buffer.oldest_index];
    uart1_tx_buffer.oldest_index++;
}

/* -------------------------------------------------------------------------- */

void uart1_tx_string(const char *string)
{
    uint8_t i = 0;

    PIE3bits.U1TXIE = 1;

    di();
    while(string[i])
    {
        if ((uart1_tx_buffer.newest_index + 1) == uart1_tx_buffer.oldest_index)
        {
            delay_ms(1);
        }

        uart1_tx_buffer.contents[uart1_tx_buffer.newest_index] = string[i++];
        uart1_tx_buffer.newest_index++;
    }
    ei();
}


/* ************************************************************************** */
