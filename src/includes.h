#ifndef INCLUDES_H
#define	INCLUDES_H

// System
#include <xc.h>

// std c headers
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// device header
#include "pic18f26k42.h"
#include "config.h"

// drivers
#include "flags.h"
#include "buttons.h"
#include "interrupt.h"
#include "delay.h"
#include "hardware.h"
#include "serial_bitbang.h"
#include "spi.h"
#include "timer.h"
#include "relays.h"
#include "controls.h"
#include "adc.h"
#include "memory.h"
#include "tuning.h"
#include "nonvolatile_memory.h"

// modules
#include "display.h"
#include "RF_sensor.h"
#include "logging_switches.h"
#include "logging.h"

#endif	/* INCLUDES_H */

