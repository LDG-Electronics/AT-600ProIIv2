#ifndef _TUNING_SEARCH_H_
#define _TUNING_SEARCH_H_

#include "relays.h"
#include "tuning_utils.h"
#include <stdint.h>

/* ************************************************************************** */

// setup
extern void tuning_search_init(void);

/* ************************************************************************** */
/*  Tuning search shapes

    These functions do the 'leg work' of tuning, crawling through the solution
    space using different search patterns.
*/

//
extern match_t inductor_sweep(tuning_errors_t *errors, match_t bestMatch,
                              uint8_t width);
//
extern match_t capacitor_sweep(tuning_errors_t *errors, match_t bestMatch,
                               uint8_t width);

//
extern match_t test_z(tuning_errors_t *errors, match_t bestMatch, uint8_t z);

//
extern match_t hiloz_tune(tuning_errors_t *errors);

//
extern match_t coarse_tune(tuning_errors_t *errors, match_t bestMatch,
                    float earlyExitThreshold);




#endif // _TUNING_SEARCH_H_