#ifndef _CONSOLE_IO_H_
#define _CONSOLE_IO_H_

/* ************************************************************************** */

// Setup
extern void console_init(void);

/* -------------------------------------------------------------------------- */

// Print a single character to the console
// Also needed for compiler provided printf
#define putch(data) UART2_tx_char(data)

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