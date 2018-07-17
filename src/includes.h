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

// device peripherals
#include "peripherals/pins.h"

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
#include "display.h"
#include "events.h"
#include "flags.h"
#include "relays.h"
#include "RF_sensor.h"
#include "ui.h"

#endif	/* _INCLUDES_H_ */