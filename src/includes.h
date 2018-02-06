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

// drivers
#include "buttons.h"
#include "interrupt.h"
#include "delay.h"
#include "hardware.h"
#include "serial_bitbang.h"
#include "spi.h"
#include "timer.h"

// modules
#include "display.h"
#include "RF_sensor.h"
#include "logging_switches.h"
#include "logging.h"

#endif	/* INCLUDES_H */

