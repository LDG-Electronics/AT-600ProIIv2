#ifndef _PRINTING_H_
#define _PRINTING_H_

/* ************************************************************************** */

// print functions for printing to the serial port
extern void putch(const char data);
extern void print(const char *string);
extern void println(const char *string);

#endif