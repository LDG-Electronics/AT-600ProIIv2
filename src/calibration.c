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
    {5.99246e-05, 0.0084949725, 0.0583486816},  // 01800000
    {5.1633e-05, 0.0078779723, 0.1638547607},   // 03500000
    {5.13935e-05, 0.0088719533, 0.0813424668},  // 07000000
    {5.44509e-05, 0.0081969944, 0.223598005},   // 10100000
    {5.34925e-05, 0.0103548966, 0.002006756},   // 14000000
    {5.25645e-05, 0.0107342809, -0.0772425672}, // 18068000
    {5.15167e-05, 0.0105767261, -0.113533868},  // 21000000
    {5.55405e-05, 0.0079429125, 0.240882941},   // 24890000
    {5.6346e-05, 0.0114048467, -0.1125376377},  // 28000000
    {8.83043e-05, 0.0083268183, 0.3591962366},  // 50000000
};

polynomial_t reverseCalibrationTable[NUM_OF_BANDS] = {
    {2.45485e-05, 0.0065363541, 0.0255649598}, // 01800000
    {2.12055e-05, 0.0063101253, 0.0243705659}, // 03500000
    {2.07476e-05, 0.0066377518, 0.0220154975}, // 07000000
    {2.24374e-05, 0.0068327958, 0.0266114382}, // 10100000
    {2.28101e-05, 0.0069334653, 0.0286353035}, // 14000000
    {2.28035e-05, 0.0069710503, 0.0287273289}, // 18068000
    {2.31275e-05, 0.0069351246, 0.0297712439}, // 21000000
    {2.47765e-05, 0.0070656526, 0.032683764},  // 24890000
    {2.35175e-05, 0.0075089686, 0.029286703},  // 28000000
    {3.12853e-05, 0.0126426893, 0.0268859307}, // 50000000
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