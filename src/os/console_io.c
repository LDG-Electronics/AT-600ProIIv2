#include "../includes.h"

/* ************************************************************************** */

void console_init(void) {
    // PPS Setup
    U2RXPPS = (PPS_PORT_D & PPS_PIN_3);
    RD2PPS = PPS_UART2_TX;

    UART2_init(_115200);
}