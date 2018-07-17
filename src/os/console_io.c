#include "../includes.h"
#include "pic18f46k42.h"

#include "../peripherals/pps.h"
#include "../peripherals/uart2.h"

/* ************************************************************************** */

void console_init(void) {
    // PPS Setup
    U2RXPPS = (PPS_PORT_D & PPS_PIN_3);
    RD2PPS = PPS_UART2_TX;

    UART2_init(_115200);
}

/* -------------------------------------------------------------------------- */

// Print a single character to the console
// Also needed for compiler provided printf
void putch(char data) { UART2_tx_char(data); }

// Read a single character from the console
char getch(void) { return UART2_rx_char(); }

// Print a string
void print(const char *string) {
    /*  Wrap UART2_tx_string() to provide both a portability layer, and to keep
        the user from having to add the termination character every time they
        print
    */
    UART2_tx_string(string, '\0');
}

// Print a string and append a newline
void println(const char *string) {
    UART2_tx_string(string, '\0');
    print("\r\n");
}