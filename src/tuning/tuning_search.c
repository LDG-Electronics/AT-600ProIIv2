#include "tuning_search.h"
#include "os/logging.h"
#include "rf_sensor.h"
#include "tuning_utils.h"
static uint8_t LOG_LEVEL = L_TRACE;

/* ************************************************************************** */

void tuning_search_init(void) {
    //
    log_register();
}
/* ************************************************************************** */
/*  Tuning search shapes

*/

// TODO: maybe use a pointer to bestMatch instead of returning a match?

/*  inductor_sweep() draws a line (2 * width) + 1, centered on bestMatch

            o = location of bestMatch

    L   |
        |       ^   --
    a   |       |    | <w>
    x   |       o   -|
    i   |       |    | <w>
    s   |       v   --
        |_________________
            C axis
*/
match_t inductor_sweep(tuning_errors_t *errors, match_t bestMatch,
                       uint8_t width) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    LOG_TRACE({ println("inductor_sweep"); });

    relays_t relays = bestMatch.relays;
    match_t match = bestMatch;

    uint8_t maxInd = calculate_max_inductor(currentRF.frequency);
    if (relays.inds < maxInd - width) {
        maxInd = relays.inds + width;
    }
    uint8_t minInd = 0;
    if (relays.inds > width) {
        minInd = relays.inds - width;
    }

    // draw the line
    relays.inds = minInd;
    while (relays.inds < maxInd) {
        relays.inds++;

        match = compare_matches(errors, relays, match);
        if (errors->any) {
            return match;
        }
    }

    LOG_INFO({
        print_comparison_count();
        println("");
    });
    return match;
}

/*  capacitor_sweep() draws a line (2 * width) + 1, centered on bestMatch

            o = location of bestMatch

    L   |
        |    ___ ___
    a   |   |<w>|<w>|
    x   |
    i   |   <---o--->
    s   |
        |_________________
            C axis
*/
match_t capacitor_sweep(tuning_errors_t *errors, match_t bestMatch,
                        uint8_t width) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    LOG_TRACE({ println("capacitor_sweep"); });

    relays_t relays = bestMatch.relays;
    match_t match = bestMatch;

    uint8_t maxCap = calculate_max_capacitor(currentRF.frequency);
    if (relays.caps < maxCap - width) {
        maxCap = relays.caps + width;
    }
    uint8_t minCap = 0;
    if (relays.caps > width) {
        minCap = relays.caps - width;
    }

    // draw the line
    relays.caps = minCap;
    while (relays.caps < maxCap) {
        relays.caps++;

        match = compare_matches(errors, relays, match);
        if (errors->any) {
            return match;
        }
    }

    LOG_INFO({
        print_comparison_count();
        println("");
    });
    return match;
}

/* -------------------------------------------------------------------------- */

const uint8_t zSteps[] = {0,  1,  2,  4,  6,  9,  12,  16,  21,  27, 34,
                          42, 51, 61, 72, 84, 97, 111, 126, 127, 127};

/*  LC_zip() draws a diagonal line starting from the origin

    L   |         /
        |       /
    a   |     /
    x   |   /
    i   | /
    s   |___________
            C axis
*/
match_t LC_zip(tuning_errors_t *errors, relays_t relays, match_t bestMatch) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    uint8_t maxCap = calculate_max_capacitor(currentRF.frequency);
    uint8_t maxInd = calculate_max_inductor(currentRF.frequency);
    match_t match = bestMatch;

    uint8_t tryIndex = 0;
    while ((zSteps[tryIndex] < maxCap) && (zSteps[tryIndex] < maxInd)) {
        relays.caps = zSteps[tryIndex];
        relays.inds = zSteps[tryIndex];

        match = compare_matches(errors, relays, match);
        if (errors->any) {
            return match;
        }

        tryIndex += 2;
    }
    return match;
}

/*  L_zip() draws a vertical line starting from <caps>

    L   |   |
        |   |
    a   |   |
    x   |   |
    i   |   |
    s   |___________
            C axis
*/
match_t L_zip(tuning_errors_t *errors, relays_t relays, match_t bestMatch,
              uint8_t caps) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    uint8_t tryIndex = 0;
    uint8_t maxInd = calculate_max_inductor(currentRF.frequency);
    match_t match = bestMatch;

    relays.caps = caps;
    while (zSteps[tryIndex] < maxInd) {
        relays.inds = zSteps[tryIndex];

        match = compare_matches(errors, relays, match);
        if (errors->any) {
            return match;
        }

        tryIndex += 2;
    }
    return match;
}

match_t test_z(tuning_errors_t *errors, match_t bestMatch, uint8_t z) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    LOG_TRACE({ println("test_z"); });

    // prepare a new relay object
    relays_t relays;
    relays.all = 0;
    relays.z = z;
    match_t match = bestMatch;

    // draw a diagonal line
    match = LC_zip(errors, relays, match);

    // draw some vertical lines
    match = L_zip(errors, relays, match, 3);
    match = L_zip(errors, relays, match, 7);

    LOG_INFO({
        print_comparison_count();
        println("");
    });

    return match;
}

/*  Do some stuff on the hi-z side and some stuff on the lo-z side and see if
    which side scores better.

    Honestly, I'm not really sure why this works.
*/
match_t hiloz_tune(tuning_errors_t *errors) {
    LOG_TRACE({ println("hiloz_tune"); });

    match_t lozMatch = test_z(errors, new_match(), 0);
    match_t hizMatch = test_z(errors, new_match(), 1);

    // return early if there are any errors
    if (errors->any) {
        return new_match();
    }

    match_t bestMatch = select_best_match(hizMatch, lozMatch);

    LOG_DEBUG({ printf("z found: %d\r\n", bestMatch.relays.z); });

    return bestMatch;
}

/* -------------------------------------------------------------------------- */

const uint8_t coarseSteps[] = {0,  1,  2,  4,  6,  9,  12,  16,  21,  27, 34,
                               42, 51, 61, 72, 84, 97, 111, 126, 127, 127};

/*  coarse_tune() searches across the entire set of possible solutions

    It uses two nested loops to cycle through capacitors, move to the next
    inductor, then cycle through capacitors, repeating this pattern across the
    entire solution set.

    L   |
        |
    a   |
    x   |
    i   |
    s   |
        |_________________
            C axis
*/
match_t coarse_tune(tuning_errors_t *errors, match_t bestMatch,
                    float earlyExitThreshold) {
    // --------------------------------------------------
    // return early if there's already an error
    if (errors->any) {
        return bestMatch;
    }
    // --------------------------------------------------

    LOG_TRACE({ println("coarse_tune"); });

    relays_t relays = bestMatch.relays;
    match_t match = bestMatch;

    uint8_t maxInd = calculate_max_inductor(currentRF.frequency);
    uint8_t maxCap = calculate_max_capacitor(currentRF.frequency);

    uint8_t inductorIndex = 0;
    while (coarseSteps[inductorIndex] < maxInd) {
        relays.inds = coarseSteps[inductorIndex++];

        uint8_t capacitorIndex = 0;
        while (coarseSteps[capacitorIndex] < maxCap) {
            relays.caps = coarseSteps[capacitorIndex++];

            match = compare_matches(errors, relays, match);
            if (errors->any) {
                return match;
            }

            if (match.matchQuality <= earlyExitThreshold) {
                LOG_DEBUG({ println("early exit!"); });
                LOG_INFO({
                    print_comparison_count();
                    println("");
                });
                return match;
            }
        }
    }

    LOG_INFO({
        print_comparison_count();
        println("");
    });
    return match;
}