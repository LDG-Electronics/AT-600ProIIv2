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
#include "peripherals/adc.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/oscillator.h"
#include "peripherals/pins.h"
#include "peripherals/pps.h"
#include "peripherals/reset.h"
#include "peripherals/spi.h"
#include "peripherals/timer.h"
#include "peripherals/uart.h"

// TuneOS
#include "os/mutex.h"
#include "os/shell.h"
#include "os/stopwatch.h"
#include "os/system_tick.h"
#include "os/event_scheduler.h"

// modules
#include "animations.h"
#include "buttons.h"
#include "calibration.h"
#include "delay.h"
#include "display.h"
#include "events.h"
#include "fast_ring_buffer.h"
#include "flags.h"
#include "frequency_counter.h"
#include "hardware.h"
#include "log_levels.h"
#include "menus.h"
#include "memory.h"
#include "meter.h"
#include "relays.h"
#include "RF_sensor.h"
#include "shell_commands.h"
#include "tuning.h"

#endif	/* _INCLUDES_H_ */