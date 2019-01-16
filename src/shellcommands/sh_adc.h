#ifndef _SH_ADC_H_
#define _SH_ADC_H_

#include <stdint.h>

/* ************************************************************************** */

extern void sh_adc(int argc, char **argv);

#define SH_ADC                                                                 \
    { sh_adc, "adc" }

/* ************************************************************************** */

#endif // _SH_ADC_H_