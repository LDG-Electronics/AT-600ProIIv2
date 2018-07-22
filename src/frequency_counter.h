#ifndef _FREQUENCY_COUNTER_H_
#define _FREQUENCY_COUNTER_H_

#include <stdint.h>

/* ************************************************************************** */

extern void frequency_counter_init(void);

/* -------------------------------------------------------------------------- */

extern uint16_t get_frequency(void);

#endif