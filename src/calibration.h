#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <stdint.h>

/* ************************************************************************** */

extern float correct_forward_power(float forward, uint16_t frequency);
extern float correct_reverse_power(float reverse, uint16_t frequency);

extern float calculate_SWR_by_watts(float forward, float reverse);

#endif