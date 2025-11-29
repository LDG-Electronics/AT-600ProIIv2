#include "tuning_utils.h"
#include "display.h"
#include "os/logging.h"
#include "rf_sensor.h"
#include <float.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void tuning_utils_init(void) {
    //
    log_register();
}

/* ************************************************************************** */

// stores the number of solutions tried
uint16_t comparisonCount;
uint16_t prevcomparisonCount;

void reset_solution_count(void) {
    comparisonCount = 0;
    prevcomparisonCount = 0;
}

/*  print_comparison_count() shows the number of tested tuning solutions

    Output: "comparisonCount: iii new: jjj"
*/
void print_comparison_count(void) {
    uint16_t difference = comparisonCount - prevcomparisonCount;

    printf("comparisonCount: %u new: %u", comparisonCount, difference);

    prevcomparisonCount = comparisonCount;
}

/* ************************************************************************** */
/*  Tuning Error Struct utils

*/
// returns a correctly initialized tuning_errors_t object
tuning_errors_t no_errors(void) {
    tuning_errors_t errors;
    errors.any = 0;
    return errors;
}

/*  Notes on tuning_followup_animation():

    This processes the error flags set during the tuning cycle.

    In theory, tuning should only allow one error to happen before exiting.
    With that in mind, this function will only display one error message.

    This decision was made in part to avoid the awful possibility of everything
    going wrong and then the tuner displaying 8 seconds of random blinks.
*/

void tuning_followup_animation(tuning_errors_t errors) {
    if (errors.any) {
        if (errors.lostRF) {
            LOG_INFO({ println("lostRF"); });

            repeat_animation(&blink_top_bar[0], 2);
        } else if (errors.noRF) {
            LOG_INFO({ println("noRF"); });

            repeat_animation(&blink_top_bar[0], 1);
        } else if (errors.noFreq) {
            LOG_INFO({ println("noFreq"); });

            repeat_animation(&blink_both_bars[0], 2);
        } else if (errors.badMatch) {
            LOG_INFO({ println("badMatch"); });

            repeat_animation(&blink_bottom_bar[0], 2);
        } else if (errors.relayError) {
            LOG_INFO({ println("relayError"); });

            repeat_animation(&toggle_inner_leds[0], 2);
        }
    }
}

/* ************************************************************************** */
/*  tuning match struct utils

*/
// returns a correctly initialized match_t object
match_t new_match(void) {
    match_t match;

    match.attemptNumber = 0;
    match.relays.all = 0;
    match.forward = 0;
    match.reverse = 0;
    match.matchQuality = FLT_MAX;

    return match;
}

/*  prints out a match_t object

    output: "(C<A>, L<B>, Z<C>, A<D>) Q: <E>, FWD: <G>, #: <H>"
*/
void print_match(match_t *match) {
    print_relays(match->relays);
    printf(" Q: %f, SWR: %f, FWD: %f, #: %u", match->matchQuality, match->swr, match->forward, match->attemptNumber);
}

/* -------------------------------------------------------------------------- */
/*  returns the 'better' of two provided match_t objects.

    'Better' is a little subjective.

    SWR would be the preferred metric, except that RF sensor limitations,
    difficulties calibrating the sensor, and math limitations mean that SWR is
    not a very reliable measurement.

    matchQuality is, essentially, a primitive analogue for SWR. It's calculated
    from the raw forward and reverse values, meaning that it is NOT subject to
    the same calibration or math limitations that SWR is.

    If the matchQualities are too close, raw forward is used as a tiebreaker.
*/
// TODO: iterate on this selection algorithm
match_t select_best_match(match_t matchA, match_t matchB) {
    if (matchA.matchQuality < matchB.matchQuality) {
        return matchA;
    } else if (matchA.matchQuality == matchB.matchQuality) {
        if (matchA.forward > matchB.forward) {
            return matchA;
        } else {
            return matchB;
        }
    } else {
        return matchB;
    }
}

/* -------------------------------------------------------------------------- */

match_t create_match_from_current_conditions(relays_t relays) {
    match_t match = new_match();

    match.attemptNumber = comparisonCount;
    match.relays = relays;
    match.forward = currentRF.forwardVolts;
    match.reverse = currentRF.reverseVolts;
    match.matchQuality = currentRF.matchQuality;
    match.swr = currentRF.swr;
    match.frequency = currentRF.frequency;

    return match;
}

/*  compare_matches()

    Publishes a a relay object, measures the resulting RF, then compares those
    measurements against the provided match object.

    Returns the better of the two matches.
*/
match_t compare_matches(tuning_errors_t *errors, relays_t relays, match_t bestMatch) {
    comparisonCount++;
    if (comparisonCount == 1000) {
        errors->timeout = 1;
        return bestMatch;
    }

    // publish our relays
    if (put_relays(relays) == -1) {
        errors->relayError = 1;
        return bestMatch;
    }

    // make sure the RF isn't going crazy
    if (!wait_for_stable_RF(1000)) {
        errors->lostRF = 1;
        return bestMatch;
    }

    measure_RF();
    calculate_watts_and_swr();

    LOG_INFO({
        print_comparison_count();
        printf(" <fwd: %f>", currentRF.forwardVolts);
        printf(" <swr: %f>", currentRF.swr);
        printf(" <q: %f>", currentRF.matchQuality);
        println("");
    });

    match_t newMatch = create_match_from_current_conditions(relays);
    match_t winner = select_best_match(newMatch, bestMatch);
    if (winner.relays.all == newMatch.relays.all) {
        LOG_DEBUG({
            print("new best: ");
            print_match(&winner);
            println("");
        });
    }

    return winner;
}

/* ************************************************************************** */
/*  Trying to apply large values of C and L at high frequencies causes extra
    stress on components, possibly leading to premature system failure.

    If the frequency is above a certain level, we'll prevent the system from
    using the highest two relays of the capacitors or inductors.

*/
#define C_LIMIT_FREQUENCY 30000 // 30mhz
uint8_t calculate_max_capacitor(uint16_t frequency) {
    if (frequency < C_LIMIT_FREQUENCY) {
        return MAX_CAPACITORS;
    } else {
        return (MAX_CAPACITORS >> 2);
    }
}

#define L_LIMIT_FREQUENCY 20000 // 20mhz
uint8_t calculate_max_inductor(uint16_t frequency) {
    if (frequency < L_LIMIT_FREQUENCY) {
        return MAX_INDUCTORS;
    } else {
        return (MAX_INDUCTORS >> 2);
    }
}