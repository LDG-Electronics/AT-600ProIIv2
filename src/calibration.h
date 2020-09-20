#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <stdint.h>

/* ************************************************************************** */

#define NUM_OF_BANDS 10
/*  Index of each band
    0 - 01800000
    1 - 03500000
    2 - 07000000
    3 - 10100000
    4 - 14000000
    5 - 18068000
    6 - 21000000
    7 - 24890000
    8 - 28000000
    9 - 50000000
*/

// Ax^2 + Bx + C
typedef struct {
    float A;
    float B;
    float C;
} polynomial_t;

extern void print_poly(polynomial_t poly);

/* -------------------------------------------------------------------------- */

extern polynomial_t forwardCalibrationTable[NUM_OF_BANDS];

extern polynomial_t reverseCalibrationTable[NUM_OF_BANDS];

/* ************************************************************************** */

extern float correct_forward_power(float forward, uint16_t frequency);
extern float correct_reverse_power(float reverse, uint16_t frequency);

extern float calculate_SWR_by_watts(float forward, float reverse);

#endif