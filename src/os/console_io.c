#include "../includes.h"

/* ************************************************************************** */

void console_init(void) {
    // PPS Setup
    U2RXPPS = (PPS_PORT_D & PPS_PIN_3);
    RD2PPS = PPS_UART2_TX;

    UART2_init(_115200);
}

/* -------------------------------------------------------------------------- */

// Needed for compiler provided printf
void putch(const char data) { UART2_putc(data); }

void print(const char *string) { UART2_tx_string(string, '\0'); }

void println(const char *string) {
    print(string);
    print("\r\n");
}