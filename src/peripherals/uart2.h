#ifndef _UART2_H_
#define _UART2_H_

/* ************************************************************************** */

#include "uart_common.h"

/* ************************************************************************** */

// Setup
extern void UART2_init(UART_baud_rates baudRate);

// Transmit a null-terminated string of any length
extern void UART2_tx_string(const char *string, const char terminator);

// Transmit a single char
extern void UART2_tx_char(char data);

// Return a single char from the RX buffer, or return 0 if the buffer is empty
extern char UART2_rx_char(void);

#endif /* _UART_H_ */