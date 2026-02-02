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
    {3.4479e-05, 0.012242772, -0.5888886198}, // 01800000
    {2.40674e-05, 0.0144357467, -1.0384081397}, // 03500000
    {2.32939e-05, 0.0121373473, -0.6636951614}, // 07000000
    {2.46085e-05, 0.0107465893, -0.3731147554}, // 10100000
    {2.36828e-05, 0.0145613211, -0.9381904235}, // 14000000
    {2.48176e-05, 0.0134516666, -0.7756303431}, // 18068000
    {2.43053e-05, 0.0138282772, -0.87842567}, // 21000000
    {2.44645e-05, 0.0146616079, -1.0958326768}, // 24890000
    {2.42895e-05, 0.013256318, -0.6386329677}, // 28000000
    {2.59059e-05, 0.0145094203, -0.6225379768}, // 50000000
};

polynomial_t reverseCalibrationTable[NUM_OF_BANDS] = {
    {7.422e-06, 0.0043297273, 0.0418036189}, // 01800000
    {5.711e-06, 0.0039180843, 0.0347077466}, // 03500000
    {5.463e-06, 0.0038557671, 0.0336771535}, // 07000000
    {5.7766e-06, 0.0039250007, 0.0378529411}, // 10100000
    {5.9187e-06, 0.0039376254, 0.0422536579}, // 14000000
    {5.7382e-06, 0.0040787178, 0.0389703097}, // 18068000
    {5.7965e-06, 0.0041264494, 0.0384298887}, // 21000000
    {6.0771e-06, 0.0042045813, 0.0442086157}, // 24890000
    {6.5335e-06, 0.004211616, 0.0441096503}, // 28000000
    {9.2993e-06, 0.0066398257, 0.0677537797}, // 50000000
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