#ifndef _INCLUDES_H_
#define	_INCLUDES_H_

/* ************************************************************************** */

// std c headers
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

// device header
#include "pic18f46k42.h"

// Project specific hardware stuff
// Must be before peripheral and OS headers
#include "hardware.h"

// device peripherals
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/pins.h"
#include "peripherals/pps.h"
#include "peripherals/relay_driver.h"
#include "peripherals/timer.h"

// TuneOS
// Must be after peripheral drivers
#include "os/buttons.h"
#include "os/console_io.h"
#include "os/log.h"
#include "os/shell/shell.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "os/event_scheduler.h"

// modules
#include "animations.h"
#include "calibration.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "frequency_counter.h"
#include "memory.h"
#include "meter.h"
#include "relays.h"
#include "RF_sensor.h"
#include "shell_commands.h"
#include "tuning.h"
#include "ui.h"

#endif	/* _INCLUDES_H_ */