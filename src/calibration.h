#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/* ************************************************************************** */

// Ax^2 + Bx + C
typedef struct {
    float A;
    float B;
    float C;
} polynomial_t;

// Two arrays, 0 is Forward, 1 is Reverse
#define NUMBER_OF_ARRAYS 2

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

extern polynomial_t calibrationBuffer[NUMBER_OF_ARRAYS][NUM_OF_BANDS];
extern const polynomial_t calibrationTable[NUMBER_OF_ARRAYS][NUM_OF_BANDS];

/* ************************************************************************** */

extern void print_poly(polynomial_t *poly);

extern uint8_t decode_frequency_to_band_index(uint16_t frequency);

#endif