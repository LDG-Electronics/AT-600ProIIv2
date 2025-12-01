#include "calibration.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/* ************************************************************************** */

void print_poly(polynomial_t poly) {
    printf("(%10.10f, %10.10f, %10.10f)", poly.A, poly.B, poly.C); //
}

/* -------------------------------------------------------------------------- */

polynomial_t forwardCalibrationTable[NUM_OF_BANDS] = {
    {1.51587e-05, -0.0060328824, 2.7082489741}, // 01800000
    {1.20743e-05, -0.0056207022, 2.7441929053}, // 03500000
    {1.11827e-05, -0.0044110833, 2.3148621334}, // 07000000
    {1.1779e-05, -0.0049848463, 2.5213582082},  // 10100000
    {1.18646e-05, -0.0049070542, 2.6061124915}, // 14000000
    {1.13141e-05, -0.00418468, 2.2563557919},   // 18068000
    {1.15549e-05, -0.0056224365, 2.9218492332}, // 21000000
    {1.13151e-05, -0.0042982617, 2.2547151272}, // 24890000
    {1.16743e-05, -0.004352165, 2.3111931975},  // 28000000
    {1.04125e-05, 0.0041391385, -0.4051507692}, // 50000000
};

polynomial_t reverseCalibrationTable[NUM_OF_BANDS] = {
    {8.7673e-06, 0.0013695546, -0.0372789455}, // 01800000
    {6.6581e-06, 0.0011797294, -0.0075654381}, // 03500000
    {6.1815e-06, 0.0013303914, -0.0230372892}, // 07000000
    {6.3509e-06, 0.0014253321, -0.0325788626}, // 10100000
    {6.3027e-06, 0.0014752539, -0.0303042619}, // 14000000
    {6.2807e-06, 0.0014321462, -0.0222443174}, // 18068000
    {6.3185e-06, 0.0013999339, -0.0286895744}, // 21000000
    {6.5617e-06, 0.0014009995, -0.0187917247}, // 24890000
    {6.9259e-06, 0.0012537864, 0.0005648386},  // 28000000
    {9.4866e-06, 0.000861187, 0.0115266885},   // 50000000
};

/* ************************************************************************** */

// TODO: perhaps these should be right in the center of their bands?
uint16_t bands[] = {
    1800, 3500, 7000, 10100, 14000, 18068, 21000, 24890, 28000, 50000,
};

int16_t find_closest_band_index(int16_t val1, int16_t val2, int16_t target) {
    if (target - bands[val1] >= bands[val2] - target) {
        return val2;
    } else {
        return val1;
    }
}

uint8_t decode_frequency_to_band_index(uint16_t frequency) {
    for (uint8_t i = 0; i < 9; i++) {
        if (frequency < bands[i + 1]) {
            return find_closest_band_index(i, i + 1, frequency);
        }
    }

    return 4;
}

/* -------------------------------------------------------------------------- */

float correct_forward_power(float forward, uint16_t frequency) {
    uint8_t band = decode_frequency_to_band_index(frequency);

    polynomial_t poly = forwardCalibrationTable[band];

    float temp = (poly.A * pow(forward, 2)) + (poly.B * forward) + poly.C;
    if (temp < 0) {
        temp = 0;
    }
    return temp;
}

float correct_reverse_power(float reverse, uint16_t frequency) {
    uint8_t band = decode_frequency_to_band_index(frequency);

    polynomial_t poly = reverseCalibrationTable[band];

    float temp = (poly.A * pow(reverse, 2)) + (poly.B * reverse) + poly.C;
    if (temp < 0) {
        temp = 0;
    }
    return temp;
}

/* -------------------------------------------------------------------------- */

/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/
float calculate_SWR_by_watts(float forward, float reverse) {
    // do not allow reverse to be greater than forward
    float tempReverse = reverse;
    if (reverse > forward) {
        tempReverse = forward - 1.0f;
    }

    // This term is used twice, so let's simplify
    float reflectionCoefficient = sqrt(tempReverse / forward);

    float swr = ((1.0f + reflectionCoefficient) / (1.0f - reflectionCoefficient));

    // Actual SWR will almost never be <1.1, but our math is poor at low SWR
    if (swr < 1.1f) {
        swr = 1.1f;
    }

    return swr;
}