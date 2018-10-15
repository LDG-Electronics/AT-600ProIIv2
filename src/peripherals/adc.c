#include "adc.h"
#include "../os/log_macros.h"
#include "fixed_voltage_reference.h"
#include "pic18f46k42.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void adc_init(void) {
    fvr_enable();

    ADREFbits.NREF = 0;    // Negative Voltage Reference, set to Vss
    ADREFbits.PREF = 0b11; // Positive Voltage Reference, set to FVR

    ADCON0bits.FM = 1; // adc result is right-justified

    ADCON0bits.CS = 0; // FOSC, divided by ADCLK
    ADCLK = 0b011111;  // FOSC/64

    ADCON0bits.ON = 1; // Enable ADC peripheral

    log_register();
}

/* -------------------------------------------------------------------------- */
/*  adc_read()

    This algorithm is designed to counteract the poorly designed RF sensor in
    legacy products. The signal is not adequately filtered, and it is not
    correctly normalized/conditioned. The sensor contains diodes with a forward
    voltage drop in the ballpark of .3V. This means that any sensor output in
    the ballpark of .3V is indistinguishable from either lack of signal or
    straight up noise.

    This issue is further compounded by the PIC ADC's inherent flaws, being a
    built-in peripheral instead of a dedicated offboard ADC IC. The above issue
    with diode forward voltage drop is compounded by the ADC's quantization
    noise. Even with no signal present, ADC conversions sometimes return
    non-zero results.

    Recent discoveries of the output of the legacy RF sensor highlight even more
    issues. Reverse or Reflected RF power is already difficult to measure due to
    the design of modern transistor radios. These radios generally(see: tentec)
    have a feature called foldback. When the radio detects poor SWR conditions,
    it lowers the output power to prevent damage to the radio's amplifier
    section.

    In ideal RF conditions(the unattainable 1:1 SWR), 100% of the energy output
    by the radio will be emitted by the antenna. In any conditions that are
    worse than perfect, some energy will not be absorbed and emitted from the
    antenna, but will instead be reflected back towards the radio.

    The problem of measuring reverse power is actually the interaction of the
    above facts:

    1) When the SWR is good, the radio will output full power, but nearly all of
       that power will be emitted by the antenna, and the reverse power will be
       miniscule and difficult to measure.

    2) When the SWR is bad, the radio's power foldback feature limits the output
       to (usually) the ballpark of 10 Watts. The reverse power is a small
       fraction of the forward power in all but the worst cases. Therefore, when
       the radio limits its output power, a small fraction of that limited
       output power then becomes, again, miniscule and difficult to measure.

    When the legacy RF sensor is monitored on an oscilloscope, some additional
    facts reveal themselves. These facts will probably be easier to document
    visually, so CHOO CHOO, here comes the ASCII train.

    Figure 1 - Good SWR, ~1.5 Watts of reverse power, ~50 Watts of forward power
    The waveform is centered on the zero axis.
    |   ___                 ___                 ___                 ___
    |  /   \               /   \               /   \               /   \
    | /     \             /     \             /     \             /     \
    |/       \           /       \           /       \           /       \
    |---------------------------------------------------------------------------
    |          \       /           \       /           \       /           \
    |           \     /             \     /             \     /             \
    |            \___/               \___/               \___/               \__
    |

    Figure 2 - Bad SWR, ~1.5 Watts of reverse power, ~25 Watts of forward power
    The waveform is still present, but now mostly exists ABOVE the zero axis.
    |      ___                 ___
    |     /   \               /   \
    |    /     \             /     \
    |   /       \           /       \
    |  /         \         /         \
    | /           \       /           \
    |----------------------------------------------
    |               \___/               \___/
    |

    Figure 3 - ADC sampling vs waveform
    An additional issue is highlighted: The PIC's ADC cannot measure negative
    voltage. From the perspective of the ADC, Figure 1 would actually look like
    this:
    |   ___                 ___                 ___                 ___
    |  /   \               /   \               /   \               /   \
    | /     \             /     \             /     \             /     \
    |/       \___________/       \___________/       \___________/       \______
    |---------------------------------------------------------------------------
    |

    Figure 4 - But wait, there's more!
    The situation is worse than it at first appears: Not only are the negative
    parts of the wave unmeasurable by the ADC, but the forward voltage drop and
    ADC quantization noise rear their heads again. Here is an artist's
    rendition of the ADC's perspective, considering all 3 phenomenon at once:
    |   ___                 ___                 ___                 ___
    |  /   \               /   \               /   \               /   \
    |_/     \___       ___/     \___       ___/     \___       ___/     \____
    |           |_____|             |_____|             |_____|             |___
    |---------------------------------------------------------------------------
    | |_____|   |_____|          |__|
    |    A         B              C

    A: The only remaining section of the original waveform
    B & C: between the diode and the forward voltage drop, it is unlikely that
           the measurable voltage will match the original waveform except with
           negative values replaced with 0. Instead, I strongly suspect that it
           will briefly hover somewhere around .3V, and then snap to 0V, then
           snap back to .3V when the voltage rises again. The exact timing of
           this is not only unknown, but perhaps also unknowable.

    The SWR sensor output is SUPPOSED to be filtered down to a clean, stable, DC
    signal. Instead, we see that it is very much still a sine wave. This wave is
    the same frequency the radio is transmitting on, EG ~1.8MHz to ~55MHz. In a
    better world, we might have a processor and ADC that is capable of sampling
    at the nyquist rate in relation to this sensor reading. If we could sample
    at >120MHz, for example, we would be able to collect enough data to
    reconstruct the waveform in abstract. It would then be possible to use
    software to do the filtering and normalizing that was SUPPOSED to be done in
    hardware.

    *deep breath*

    Since we are, in fact, limited by the reality of our physical existance, we
    need to cut this gordian knot with the extremely tiny knife provided by an
    8-bit PIC microcontroller.

    Due to what is essentially a random act of muse, a solution revealed itself.

    Collect and average 16 ADC samples. Only collect samples that are > 1. Count
    the number of discarded samples. Results are returned in a struct that
    contains both the average and the number of discarded samples.

    The number of discarded samples can then be used as a measurement quality
    indicator. If no samples were discarded, then the result can probably be
    taken at face value. If a low number (10 to 50, perhaps) samples were
    discarded, perhaps the measurement should be taken again. A high number of
    discarded samples(200 to 255) should be taken as a sign that funny business
    is happening.
*/

/* -------------------------------------------------------------------------- */

#define NUM_OF_CHANNELS 2
#define _1024mV FVR_GAIN_1X
#define _2048mV FVR_GAIN_2X
#define _4096mV FVR_GAIN_4X

//! Warning - input not sanitized
static void adc_select_channel(uint8_t channel) { ADPCH = channel; }

//! Warning - input not sanitized
static void adc_select_scale(uint8_t scale) {
    fvr_set_adc_buffer_gain(scale);

    // wait for FVR to stabilize
    fvr_wait_until_ready();
}

// For power users who know the idiosyncracies of this platform and really want
// only one raw sample.
uint16_t adc_single_sample(uint8_t channel) {
    adc_select_channel(channel);
    adc_select_scale(FVR_GAIN_4X);

    // Engage
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO) {
        // Wait for the conversion to finish
    }

    return ADRES;
}

/* -------------------------------------------------------------------------- */

#define NUMBER_OF_SAMPLES 8
static uint16_t samplebuffer[NUMBER_OF_SAMPLES];

static void clear_sample_buffer(void) {
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        samplebuffer[i] = 0;
    }
}

static void print_sample_buffer(void) {
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        printf("(%d, %d)\r\n", i, samplebuffer[i]);
    }
}

static adc_result_t adc_average_of_valid_samples(uint8_t channel) {
    LOG_TRACE({ println("adc_average_of_valid_samples"); });

    adc_select_channel(channel);
    clear_sample_buffer();

    adc_result_t result;
    uint16_t totalSamples = 0;
    uint8_t samples = 0;

    while (samples < NUMBER_OF_SAMPLES) {
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO) {
            // Wait for the conversion to finish
        }

        if (ADRES != 0) { // samples of 0 are invalid
            samplebuffer[samples++] = ADRES;
        }

        totalSamples++;
        if (totalSamples == 255) {
            break;
        }
    }

    result.discardedSamples = totalSamples - NUMBER_OF_SAMPLES;

    double sum = 0;
    for (uint8_t i = 0; i < NUMBER_OF_SAMPLES; i++) {
        sum += samplebuffer[i];
    }
    result.value = sum / NUMBER_OF_SAMPLES;

    return result;
}

// This is the ADC call that produces a definitely good value for someone who
// doesn't know all the idiosyncracies of this platform.
adc_result_t adc_read(uint8_t channel) {
    LOG_TRACE({ println("adc_read"); });

    static uint8_t maxVoltage[NUM_OF_CHANNELS] = {_4096mV, _1024mV};
    adc_result_t measurement;

    for (uint8_t i = 0; i < 3; i++) {
        LOG_DEBUG({
            printf("channel: %d, maxVoltage: %d\r\n", channel,
                   maxVoltage[channel]);
        });
        adc_select_scale(maxVoltage[channel]);
        measurement = adc_average_of_valid_samples(channel);

        LOG_DEBUG({
            println("");
            print_sample_buffer();
            printf("totalSamples: %d\r\n", measurement.discardedSamples);
            printf("average: %f\r\n", measurement.value);
        });
        // If necessary, adjust maxVoltage of ADC and resample.
        // We want to keep maxVoltage as low as possible so we get the best
        // accuracy.
        if (measurement.value < 1750) { // TODO - magic number
            // if measurement fits in next lowest voltage range
            if (maxVoltage[channel] > _1024mV) {
                LOG_INFO({ println("decreasing maxVoltage"); });
                maxVoltage[channel]--;
            } else {
                break; // can't decrease maxVoltage because we're in the lowest
                       // voltage range available
            }
        } else if (measurement.value > 3900) { // TODO - magic number
            // if measurement is approaching the top of the current voltage
            // range
            if (maxVoltage[channel] < _4096mV) {
                LOG_INFO({ println("increasing maxVoltage"); });
                maxVoltage[channel]++;
            } else {
                break; // can't increase maxVoltage because we're in the highest
                       // voltage range available
            }
        } else {
            break; // measurement isn't bad, so it must be good!
        }
    }

    // Convert measurement to millivolts as needed.
    if (maxVoltage[channel] == _1024mV) {
        measurement.value /= 4;
    } else if (maxVoltage[channel] == _2048mV) {
        measurement.value /= 2;
    }

    return measurement;
}