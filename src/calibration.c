#include "calibration.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

void print_poly(polynomial_t *poly) {
    printf("A = %f\r\n", poly->A);
    printf("B = %f\r\n", poly->B);
    printf("C = %f\r\n", poly->C);
}

/* -------------------------------------------------------------------------- */

polynomial_t const forwardCalibrationTable[NUM_OF_BANDS] = {
    {3.6555900444132165e-05, -0.0015434445883156646, 1.1854880579097262},
    {2.5317605410047606e-05, 0.006997064709869251, -0.004701911882182358},
    {2.3403776976021854e-05, 0.007613737879580927, -0.71551775684892},
    {2.3029223082739753e-05, 0.009321865127722014, -1.3546754296805104},
    {2.4059957136740695e-05, 0.008217050733999972, -1.5725247784108753},
    {2.2889871082837078e-05, 0.010194111855605927, -1.8283728591503319},
    {2.384082506217916e-05, 0.007167791694290791, -0.4207794062784067},
    {2.383799723119104e-05, 0.00729782635402835, -0.1999997254941914},
    {2.422862399481827e-05, 0.008306640046400212, -0.21205320686210774},
    {2.7800555242304257e-05, -0.0018858949717490464, 1.33829257612857},
};

polynomial_t const reverseCalibrationTable[NUM_OF_BANDS] = {
    {-1.801249242849359e-05, 0.036838586207513506, -4.449896887085058},
    {5.484104509420962e-06, 0.009375712494145608, -1.1473144100057233},
    {5.050950818723708e-06, 0.0061173382318063995, -0.20301000809268502},
    {2.380746725581127e-06, 0.008265949205077533, -0.6136746689133418},
    {1.0557231955521396e-06, 0.01185045958388053, -1.8922853550621752},
    {8.258942157754763e-06, 0.002980546141120756, -0.14912318493463914},
    {5.322662159945952e-06, 0.006970977937226436, -0.4727729706895616},
    {-1.086653311965273e-06, 0.014073038815934998, -1.2471682606089445},
    {6.671787027988834e-06, 0.0034486699500952877, 0.5136661048401308},
    {-7.981231630222142e-09, 0.005714639848149372, 0.9335107105151152},
};

/* ************************************************************************** */

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
    float x = sqrt(reverse / forward);
    return ((1.0 + x) / (1.0 - x));
}