#ifndef _CALIBRATION_H_
#define _CALIBRATION_H_

#include <float.h>
#include <math.h>
#include <stdlib.h>

/* ************************************************************************** */

// Ax^2 + Bx + C
typedef struct {
    double A;
    double B;
    double C;
} polynomial_s;

extern polynomial_s fPoly;
extern polynomial_s rPoly;

// Two arrays, 0 is Forward, 1 is Reverse
#define NUMBER_OF_ARRAYS 2

/*  Frequency of each slot:
    0 -
    1 -
    2 -
    3 -
    4 -
    5 -
    6 -
    7 -
    8 -
    9 -
*/
#define NUMBER_OF_SLOTS 10

extern polynomial_s calibrationTable[NUMBER_OF_ARRAYS][NUMBER_OF_SLOTS];

/* ************************************************************************** */

extern void print_poly(polynomial_s *poly);

#endif