#ifndef _USB_H_
#define _USB_H_

#include "peripherals/uart.h"

/* ************************************************************************** */

// initialize the USB port by creating and passing in a UART interface object
extern void usb_init(uart_interface_t interface);

// call this often to service the USB port
extern void usb_update(void);

#endif // _USB_H_