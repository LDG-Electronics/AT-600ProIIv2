#ifndef _CONSOLE_IO_H_
#define _CONSOLE_IO_H_

/* ************************************************************************** */

extern void console_init(void);

/* -------------------------------------------------------------------------- */

// print functions for printing to the serial port
extern void putch(const char data);
extern void print(const char *string);
extern void println(const char *string);

#endif