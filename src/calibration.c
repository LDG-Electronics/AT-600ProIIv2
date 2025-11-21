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
    {1.66369e-05, -0.0129969656, 9.2777270821}, // 01800000
    {1.32475e-05, -0.0118276036, 9.3359873564}, // 03500000
    {1.21448e-05, -0.0095647727, 7.8649182217}, // 07000000
    {1.28787e-05, -0.0107723887, 8.6461210559}, // 10100000
    {1.29665e-05, -0.0107108898, 8.7456662803}, // 14000000
    {1.22895e-05, -0.0093242284, 7.6995079113}, // 18068000
    {1.2829e-05, -0.0122921889, 9.9281450704},  // 21000000
    {1.22611e-05, -0.0093680449, 7.731078905},  // 24890000
    {1.2749e-05, -0.0099401977, 8.1602561691},  // 28000000
    {9.8559e-06, 0.0062886553, -2.0681563758},  // 50000000
};

polynomial_t reverseCalibrationTable[NUM_OF_BANDS] = {
    {0, 0.017848632324313465, -0.5292995055154676},     // 01800000
    {0, 0.004673072945827411, -0.732292679155033},      // 03500000
    {0, 1.06767900366698e-05, -0.004300795370556416},   // 07000000
    {0, 0.0006729386554650659, -0.21781240582551664},   // 10100000
    {0, 0.006525592469784352, -0.886250599432716},      // 14000000
    {0, 0.0015890927616966878, -0.33453007517330163},   // 18068000
    {0, 4.551710001370734e-07, -0.0001153122634817359}, // 21000000
    {0, 0.001047782896889901, -0.6095640520480708},     // 24890000
    {0, 0.002630555791028688, -1.4607997497489948},     // 28000000
    {0, 0.0029554812588032543, -0.08989538158119141},   // 50000000
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

    return (poly.A * pow(forward, 2)) + (poly.B * forward) + poly.C;
}

float correct_reverse_power(float reverse, uint16_t frequency) {
    uint8_t band = decode_frequency_to_band_index(frequency);

    polynomial_t poly = reverseCalibrationTable[band];

    return (poly.A * pow(reverse, 2)) + (poly.B * reverse) + poly.C;
}

/* -------------------------------------------------------------------------- */

/*  SWR calculation

    SWR = (1 + sqrt(Pr/Pf))/(1 - sqrt(Pr/Pf))
*/
float calculate_SWR_by_watts(float forward, float reverse) {
    // This term is used twice, so let's simplify
    float reflectionCoefficient = sqrt(reverse / forward);

    float swr = ((1.0 + reflectionCoefficient) / (1.0 - reflectionCoefficient));

    // Actual SWR will almost never be <1.1, but our math is poor at low SWR
    if (swr < 1.1) {
        swr = 1.1;
    }

    return swr;
}