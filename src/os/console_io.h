#ifndef _CONSOLE_IO_H_
#define _CONSOLE_IO_H_

/* ************************************************************************** */

/*  Notes on serial speeds:

    TODO: investigate whether switching to 230,400b or 460,800b are possible

    This system works best(so far) at 115,200 baud.

    At 115,200 baud, each bit takes 8.6805555555556 uS.

    The UART is configured for 1 start bit, 8 data bits, and 0 stop bits.

    One char therefore requires 9 bits.

    9bits * 8.68uS = 78.12uS
    1mS = 1000uS
    1000uS / 78.12uS = 12.8 chars/mS

    If assume that transmission isn't 100% continuous, and account for the
    interrupt latency between one byte finishing and the next being loaded, then
    we can assume that the console throughput is roughly 10 chars per mS.

    Expressed in other terms:

    10c/mS * 10mS = 100 c/10mS
    10c/mS * 100mS = 1,000 c/100mS
    10c/mS * 1000mS = 10,000 c/S

    These throughput rates are important for estimating the interaction between
    serial transmissions and system delays.
*/

#include "../peripherals/uart2.h"

// Setup
extern void console_init(void);

/* -------------------------------------------------------------------------- */

// Print a single character to the console
// Also needed for compiler provided printf
// #define putch(data) UART2_tx_char(data)
extern void putch(char data);

// Read a single character from the console
#define getch() UART2_rx_char()

// Print a string
#define print(string) UART2_tx_string(string, '\0')

// Print a string and append a newline
#define println(string)                                                        \
    do {                                                                       \
        print(string);                                                         \
        print("\r\n");                                                         \
    } while (0);

#endif