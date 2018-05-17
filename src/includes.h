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

// modules
#include "adc.h"
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
#include "nonvolatile_memory.h"
#include "pins.h"
#include "pps.h"
#include "relays.h"
#include "RF_sensor.h"
#include "shell.h"
#include "spi.h"
#include "stopwatch.h"
#include "system_tick.h"
#include "tasks.h"
#include "timer.h"
#include "tuning.h"
#include "uart.h"

#endif	/* _INCLUDES_H_ */