#include "serial_port.h"
#include "../hardware.h"
#include "../peripherals/pps.h"
#include "../peripherals/uart1.h"
#include "../peripherals/uart2.h"

/* ************************************************************************** */

#ifndef SERIAL_PORT_BAUD
#error "SERIAL_PORT_BAUD is not defined"
#endif

/* -------------------------------------------------------------------------- */

#ifndef SERIAL_PORT_UART_NUMBER
#error "SERIAL_PORT_UART_NUMBER is not defined"
#endif

#define UART_RX_PPS_CONCAT(uartNum, rxPin) U##uartNum##RXPPS = rxPin;
#define UART_RX_PPS_EXPAND(uartNum, rxPin) UART_RX_PPS_CONCAT(uartNum, rxPin)
#define UART_RX_PPS(rxPin) UART_RX_PPS_EXPAND(SERIAL_PORT_UART_NUMBER, rxPin)

#define UART_TX_PPS_CONCAT(uartNum, txPin) txPin = PPS_UART##uartNum##_TX;
#define UART_TX_PPS_EXPAND(uartNum, txPin) UART_TX_PPS_CONCAT(uartNum, txPin)
#define UART_TX_PPS(txPin) UART_TX_PPS_EXPAND(SERIAL_PORT_UART_NUMBER, txPin)

#define UART_INIT_CONCAT(uartNum, baud) UART##uartNum##_init(baud)
#define UART_INIT_EXPAND(uartNum, baud) UART_INIT_CONCAT(uartNum, baud)
#define UART_INIT(baud) UART_INIT_EXPAND(SERIAL_PORT_UART_NUMBER, baud)

#define UART_RX_CHAR_CONCAT(uartNum) UART##uartNum##_rx_char()
#define UART_RX_CHAR_EXPAND(uartNum) UART_RX_CHAR_CONCAT(uartNum)
#define UART_RX_CHAR() UART_RX_CHAR_EXPAND(SERIAL_PORT_UART_NUMBER)

#define UART_TX_CHAR_CONCAT(uartNum, data) UART##uartNum##_tx_char(data)
#define UART_TX_CHAR_EXPAND(uartNum, data) UART_TX_CHAR_CONCAT(uartNum, data)
#define UART_TX_CHAR(data) UART_TX_CHAR_EXPAND(SERIAL_PORT_UART_NUMBER, data)

#define UART_TX_STRING_CONCAT(uartNum, string, term)                           \
    UART##uartNum##_tx_string(string, term)
#define UART_TX_STRING_EXPAND(uartNum, string, term)                           \
    UART_TX_STRING_CONCAT(uartNum, string, term)
#define UART_TX_STRING(string, term)                                           \
    UART_TX_STRING_EXPAND(SERIAL_PORT_UART_NUMBER, string, term)

/* ************************************************************************** */

void serial_port_init(pps_input_t rxPin, pps_output_t *txPin) {
    // PPS Setup
    UART_RX_PPS(rxPin);
    UART_TX_PPS(*txPin);

    UART_INIT(SERIAL_PORT_BAUD);
}

/* -------------------------------------------------------------------------- */

// Print a single character to the console
// Also needed for compiler provided printf
void putch(char data) { UART_TX_CHAR(data); }

// Read a single character from the console
char getch(void) { return UART_RX_CHAR(); }

// Print a string
void print(const char *string) {
    /*  Wrap UART_TX_STRING() to provide both a portability layer, and to keep
        the user from having to add the termination character every time they
        print
    */
    UART_TX_STRING(string, '\0');
}

// Print a string and append a newline
void println(const char *string) {
    UART_TX_STRING(string, '\0');
    print("\r\n");
}