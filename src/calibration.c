#include "calibration.h"
#include <stdio.h>

/* ************************************************************************** */

polynomial_s fPoly = {2.6294242259022297e-005, 1.6324165531607232e-003,
                      2.0595782264652627e+000};

polynomial_s rPoly = {6.8404355873571254e-006, -2.1771708814267987e-005,
                      5.4278972452307039e-001};

polynomial_s polyArray[10];

polynomial_s calibrationTable[NUMBER_OF_ARRAYS][NUMBER_OF_SLOTS];

/* ************************************************************************** */

extern void print_poly(polynomial_s *poly) {
    printf("A = %f\r\n", poly->A);
    printf("B = %f\r\n", poly->B);
    printf("C = %f\r\n", poly->C);
}