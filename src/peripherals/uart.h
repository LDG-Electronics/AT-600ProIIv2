#ifndef _UART_H_
#define	_UART_H_

/* ************************************************************************** */

enum baud_rates {_2400, _4800, _9600, _10417, _19200, _38400, _115200};

/* ************************************************************************** */

// Setup
extern void UART1_init(enum baud_rates baudRate);

// Transmit a null-terminated string of any length
extern void UART1_tx_string(const char *string, const char terminator);

// Transmit a single char
extern void UART1_tx_char(char data);

// Return a single char from the RX buffer, or return 0 if the buffer is empty
extern char UART1_rx_char(void);

/* ************************************************************************** */

// Setup
extern void UART2_init(enum baud_rates baudRate);

// Transmit a null-terminated string of any length
extern void UART2_tx_string(const char *string, const char terminator);

// Transmit a single char
extern void UART2_tx_char(char data);

// Return a single char from the RX buffer, or return 0 if the buffer is empty
extern char UART2_rx_char(void);


#endif /* _UART_H_ */