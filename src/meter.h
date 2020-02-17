#ifndef _METER_H_
#define _METER_H_

/* ************************************************************************** */

#include "peripherals/uart.h"

/* ************************************************************************** */

extern void meter_init(uart_interface_t interface);

extern void attempt_meter_update(void);

#endif