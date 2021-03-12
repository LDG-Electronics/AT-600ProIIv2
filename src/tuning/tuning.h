#ifndef _TUNING_H_
#define _TUNING_H_

#include <stdint.h>
#include "tuning_utils.h"

/* ************************************************************************** */

// setup
extern void tuning_init(void);

/* -------------------------------------------------------------------------- */
/*  Tuning

*/

// attempts to tune, without using memories
extern tuning_errors_t full_tune(void);

// attempts to look up a stored memory that matches the current frequency
extern tuning_errors_t memory_tune(void);

#endif // _TUNING_H_