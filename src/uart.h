#ifndef _UART_H_
#define	_UART_H_

/* ************************************************************************** */

extern void uart1_init(void);

/* -------------------------------------------------------------------------- */

extern void uart1_tx_char(const char data);
extern void uart1_tx_string(const char *string);

#endif /* _UART_H_ */