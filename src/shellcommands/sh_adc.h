#ifndef _SH_ADC_H_
#define _SH_ADC_H_

#include <stdint.h>

/* ************************************************************************** */

extern void adc(int argc, char **argv);

#define SH_ADC                                                                 \
    { adc, "adc" }

/* ************************************************************************** */

#endif // _SH_ADC_H_