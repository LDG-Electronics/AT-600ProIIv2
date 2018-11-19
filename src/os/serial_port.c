#include "serial_port.h"
#include "../peripherals/pps.h"
#include "../peripherals/uart2.h"

/* ************************************************************************** */

void serial_port_init(pps_input_t rxPin, pps_output_t *txPin) {
    // PPS Setup
    U2RXPPS = rxPin;
    *txPin = PPS_UART2_TX;

    UART2_init(_115200);
}

/* -------------------------------------------------------------------------- `*/

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