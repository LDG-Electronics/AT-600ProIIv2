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
    {4.07329851293587e-05, 0.006237642505791723, 0.8714410956306531},     // 01800000
    {2.7813792723010362e-05, 0.004355978365610147, 0.6133836776106993},   // 03500000
    {2.2493517724510557e-05, 0.0014376398899573113, 1.1659109434249857},  // 07000000
    {2.088226109121096e-05, 1.8207595316892903e-05, 1.9337604387027232},  // 10100000
    {2.000173919863571e-05, 0.002052336406474602, 0.6859328150428368},    // 14000000
    {2.014952098052724e-05, 0.00024412512341215548, 2.1307101055126245},  // 18068000
    {1.9787262055684338e-05, -0.0009133275703805309, 2.0045459699621704}, // 21000000
    {1.7979333951574065e-05, 8.91487189739601e-05, 2.3540454581951495},   // 24890000
    {1.9148980054694812e-05, 0.001261300261576872, 0.9251201401619175},   // 28000000
    {2.0125743039922845e-05, 0.007492758865760548, -1.5395144637011373},  // 50000000
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