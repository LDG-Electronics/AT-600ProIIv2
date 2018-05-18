#ifndef _INCLUDES_H_
#define	_INCLUDES_H_

/* ************************************************************************** */

// std c headers
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <math.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

// device header
#include "pic18f46k42.h"

// device peripherals
#include "adc.h"
#include "nonvolatile_memory.h"
#include "pps.h"
#include "spi.h"
#include "system_tick.h"
#include "timer.h"
#include "uart.h"

// modules
#include "animations.h"
#include "buttons.h"
#include "calibration.h"
#include "delay.h"
#include "display.h"
#include "events.h"
#include "fast_ring_buffer.h"
#include "flags.h"
#include "hardware.h"
#include "log_levels.h"
#include "menus.h"
#include "memory.h"
#include "meter.h"
#include "pins.h"
#include "relays.h"
#include "RF_sensor.h"
#include "shell.h"
#include "stopwatch.h"
#include "tasks.h"
#include "tuning.h"

#endif	/* _INCLUDES_H_ */