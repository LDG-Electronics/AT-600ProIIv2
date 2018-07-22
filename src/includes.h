#ifndef _INCLUDES_H_
#define	_INCLUDES_H_

/* ************************************************************************** */

// std c headers
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

// device header
#include "pic18f46k42.h"

// Project specific hardware stuff
// Must be before peripheral and OS headers
#include "hardware.h"

// TuneOS
// Must be after peripheral drivers
#include "os/console_io.h"
#include "os/shell/shell.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "os/event_scheduler.h"

// modules
#include "animations.h"
#include "calibration.h"
#include "display.h"
#include "flags.h"
#include "relays.h"
#include "rf_sensor.h"

#endif	/* _INCLUDES_H_ */