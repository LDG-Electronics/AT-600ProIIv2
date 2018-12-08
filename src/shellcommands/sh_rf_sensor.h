#ifndef _SH_RF_SENSOR_H_
#define _SH_RF_SENSOR_H_

#include <stdint.h>

/* ************************************************************************** */

extern void calibration_packet(int argc, char **argv);

#define SH_CAL                                                                 \
    { calibration_packet, "cal" }

/* ************************************************************************** */

#endif