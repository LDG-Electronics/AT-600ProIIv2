#include "tuning.h"
#include "calibration.h"
#include "display.h"
#include "flags.h"
#include "memory.h"
#include "os/log_macros.h"
#include "os/system_time.h"
#include "relays.h"
#include "rf_sensor.h"
#include <float.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void tuning_init(void) { log_register(); }

/* ************************************************************************** */

// File scope: stores the number of solutions tried
static uint16_t comparisonCount;
static uint16_t prevcomparisonCount;

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

typedef struct {
    uint16_t attemptNumber;
    relays_t relays;
    float forward;
    float reverse;
    float matchQuality;
    float swr;
} match_t;

// returns a correctly initialized match_t object
match_t new_match(void) {
    match_t match;

    match.attemptNumber = 0;
    match.relays.all = 0;
    match.forward = 0;
    match.reverse = 0;
    match.matchQuality = FLT_MAX;
    match.swr = FLT_MAX;

    return match;
}

/*  prints out a match_t object

    output: "(C<A>, L<B>, Z<C>, A<D>) Q: <E>, SWR: <F>, FWD: <G>, #: <H>"
*/
void print_match(match_t *match) {
    print_relays(match->relays);
    printf(" Q: %f, SWR: %f, FWD: %f, #: %u", match->matchQuality, match->swr,
           match->forward, match->attemptNumber);
}

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

/* ************************************************************************** */

match_t create_match_from_current_conditions(relays_t relays) {
    match_t match = new_match();

    match.attemptNumber = comparisonCount;
    match.relays = relays;
    match.forward = currentRF.forward;
    match.reverse = currentRF.reverse;
    match.matchQuality = currentRF.matchQuality;
    match.swr = currentRF.swr;

    return match;
}

/*  compare_matches()

    Publishes a a relay object, measures the resulting RF, then compares those
    measurements against the provided match object.

    Returns the better of the two matches.
*/
match_t compare_matches(tuning_errors_t *errors, relays_t relays,
                        match_t bestMatch) {
    comparisonCount++;

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

/* -------------------------------------------------------------------------- */

#define C_LIMIT_FREQUENCY 30000 // 30mhz
uint8_t calculate_max_capacitor(void) {
    if (currentRF.frequency < C_LIMIT_FREQUENCY) {
        return MAX_CAPACITORS;
    } else {
        return (MAX_CAPACITORS >> 2);
    }
}

#define L_LIMIT_FREQUENCY 20000 // 20mhz
uint8_t calculate_max_inductor(void) {
    if (currentRF.frequency < L_LIMIT_FREQUENCY) {
        return MAX_INDUCTORS;
    } else {
        return (MAX_INDUCTORS >> 2);
    }
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

    uint8_t maxCap = calculate_max_capacitor();
    uint8_t maxInd = calculate_max_inductor();
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
    uint8_t maxInd = calculate_max_inductor();
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

    uint8_t maxInd = calculate_max_inductor();
    uint8_t maxCap = calculate_max_capacitor();

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

/* -------------------------------------------------------------------------- */

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

    uint8_t maxInd = calculate_max_inductor();
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

    uint8_t maxCap = calculate_max_capacitor();
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

tuning_errors_t no_errors(void) {
    tuning_errors_t errors;
    errors.any = 0;
    return errors;
}

/* ************************************************************************** */

tuning_errors_t full_tune(void) {
    LOG_TRACE({ println("full_tune"); });

    tuning_errors_t errors = no_errors();
    reset_solution_count();

    // early exit if there's no RF
    if (!wait_for_stable_RF(2500)) {
        errors.noRF = 1;
        return errors;
    }
    measure_frequency();
    LOG_DEBUG({ printf("frequency: %u KHz\r\n", currentRF.frequency); });

    // prepare match objects
    match_t bestMatch = new_match();
    match_t bypassMatch = compare_matches(&errors, bypassRelays, new_match());
    LOG_DEBUG({
        print("bypassMatch: ");
        print_match(&bypassMatch);
        println("");
    });

    // identify the correct hi/lo z setting
    bestMatch = hiloz_tune(&errors);

    // wide grid search
    bestMatch = coarse_tune(&errors, bestMatch, (bypassMatch.matchQuality / 2));

    //
    bestMatch = inductor_sweep(&errors, bestMatch, 10);
    bestMatch = capacitor_sweep(&errors, bestMatch, 10);

    // maybe we have the wrong Z
    // TODO: shouldn't this be checking caps?
    if (bestMatch.relays.inds < 3) {
        bestMatch = test_z(&errors, bestMatch, !bestMatch.relays.z);
    }

    //
    bestMatch = inductor_sweep(&errors, bestMatch, 10);
    bestMatch = capacitor_sweep(&errors, bestMatch, 10);

    //
    bestMatch = inductor_sweep(&errors, bestMatch, 5);
    bestMatch = capacitor_sweep(&errors, bestMatch, 5);
    bestMatch = inductor_sweep(&errors, bestMatch, 5);
    bestMatch = capacitor_sweep(&errors, bestMatch, 5);

    // errors during tuning will fall through to this point
    if (errors.any) {
        return errors;
    }

    // re-publish the final results
    if (put_relays(bestMatch.relays) == -1) {
        errors.relayError = 1;
        return errors;
    }

    // exit if there's no RF
    if (!wait_for_stable_RF(500)) {
        errors.lostRF = 1;
        return errors;
    }

    // measure the RF one last time
    measure_RF();
    calculate_watts_and_swr();

    // Save the result, if it's good enough
    LOG_INFO({ printf("final SWR: %f\r\n", currentRF.swr); });
    if (currentRF.swr < 1.8) {
        NVM_address_t address = convert_memory_address(currentRF.frequency);
        if (address) {
            LOG_DEBUG({ printf("saving memory @ %u\r\n", address); });
            memory_store(address, bestMatch.relays);
        }
    }
    return errors;
}

/* -------------------------------------------------------------------------- */

#define MEMORY_WIDTH 2
#define NUM_OF_MEMORIES 9
#define MAXIMUM_RECALL_ATTEMPTS 20

tuning_errors_t memory_tune(void) {
    LOG_TRACE({ println("memory_tune"); });

    tuning_errors_t errors = no_errors();
    reset_solution_count();

    // If we fail to find RF, then set an error and exit.
    if (!wait_for_stable_RF(2500)) {
        errors.noRF = 1;
        return errors;
    }
    measure_frequency();

    LOG_DEBUG({ printf("frequency: %u KHz\r\n", currentRF.frequency); });

    // prepare the address
    NVM_address_t address = convert_memory_address(currentRF.frequency);
    if (!address) {
        errors.noMemory = 1;
        return errors;
    }

    // Create and initialize local storage for recalled memories
    relays_t memoryBuffer[NUM_OF_MEMORIES];
    for (uint8_t i = 0; i < NUM_OF_MEMORIES; i++) {
        memoryBuffer[i].all = 0;
    }

    // Use this to count how many memories are recalled
    uint8_t memoriesFound = 0;

    // --------------------------------------------------
    /*  memory recall section, theory of operation

        memory_recall() returns the relay object found at the provided address.
        The 'ant' field of a stored memory is hijacked into a flag that
        represents whether a memory has ever been written.

        1 means no memory, 0 means yes memory.

        The hypothesis is that over the life of the tuner, most memory slots
        will probably never be written to. When we read a memory out into
        tempRelays


    */

    relays_t tempRelays = memory_recall(address);
    if (tempRelays.ant == 0) {
        LOG_DEBUG({ println("successfully recalled memory"); });
        memoryBuffer[memoriesFound++] = tempRelays;
    }

    uint8_t memoryOffset = MEMORY_WIDTH;
    uint8_t recallAttempts = 0;
    while (1) {
        tempRelays = memory_recall(address + memoryOffset);
        if (tempRelays.ant == 0) {
            LOG_DEBUG({ println("successfully recalled memory"); });
            memoryBuffer[memoriesFound++] = tempRelays;
        }
        if (memoriesFound == NUM_OF_MEMORIES) {
            break;
        }

        tempRelays = memory_recall(address - memoryOffset);
        if (tempRelays.ant == 0) {
            LOG_DEBUG({ println("successfully recalled memory"); });
            memoryBuffer[memoriesFound++] = tempRelays;
        }
        if (memoriesFound == NUM_OF_MEMORIES) {
            break;
        }

        // increase the memoryOffset for the next time through the loop
        memoryOffset += MEMORY_WIDTH;

        // make sure we don't recall memories forever
        recallAttempts++;
        if (recallAttempts == MAXIMUM_RECALL_ATTEMPTS) {
            break;
        }
    }

    // --------------------------------------------------

    // if we didn't successfully recall a memory, set an error and exit
    if (!memoriesFound) {
        errors.noMemory = 1;
        return errors;
    }

    LOG_DEBUG({
        println("Printing memoryBuffer:");
        printf("Recalled %u memories", memoriesFound);
        for (uint8_t i = 0; i < memoriesFound; i++) {
            printf("# %u: ", i);
            print_relays(memoryBuffer[i]);
            println("");
        }
    });

    // Let's test all the memories we recalled
    match_t bestMatch = new_match();
    for (uint8_t i = 0; i < memoriesFound; i++) {
        bestMatch = compare_matches(&errors, memoryBuffer[i], bestMatch);
        if (errors.any) {
            return errors;
        }
    }

    // re-publish the final results
    if (put_relays(bestMatch.relays) == -1) {
        errors.relayError = 1;
        return errors;
    }

    // exit if there's no RF
    if (!wait_for_stable_RF(250)) {
        errors.lostRF = 1;
        return errors;
    }

    // measure the RF one last time
    measure_RF();
    calculate_watts_and_swr();

    // Did we find a valid memory?
    LOG_INFO({ printf("final SWR: %f\r\n", currentRF.swr); });
    if (currentRF.swr < 1.7) {
        LOG_INFO({
            printf("found memory: %f ", currentRF.swr);
            print_relays(currentRelays[systemFlags.antenna]);
            println("");
        });
        // returning with no errors means success
        return errors;
    }

    // We must not have found a valid memory
    errors.noMemory = 1;
    return errors;
}

/* -------------------------------------------------------------------------- */

/*  Notes on tuning_followup_animation():

    This processes the error flags set during the tuning cycle.

    In theory, tuning should only allow one error to happen before exiting.
    With that in mind, this function will only display one error message.

    This decision was made in part to avoid the awful possibility of everything
    going wrong and then the tuner displaying 8 seconds of random blinks.
*/
void tuning_followup_animation(tuning_errors_t errors) {
    display_clear();

    if (errors.any) {
        if (errors.lostRF) {
            LOG_ERROR({ println("lostRF"); });

            repeat_animation(&blink_top_bar[0], 2);
        } else if (errors.noRF) {
            LOG_ERROR({ println("noRF"); });

            repeat_animation(&blink_top_bar[0], 1);
        } else if (errors.relayError) {
            LOG_ERROR({ println("relayError"); });

            repeat_animation(&toggle_outer_leds[0], 3);
        }
    } else {
        if (currentRF.swr < 1.7) {
            LOG_INFO({ println("good SWR"); });

            play_animation(&center_crawl[0]);
        } else if (currentRF.swr < 3.0) {
            LOG_INFO({ println("fair SWR"); });

            // TODO: pick the right animation
            play_animation(&center_crawl[0]);
        } else if (currentRF.swr >= 3.0) {
            LOG_INFO({ println("bad SWR"); });

            repeat_animation(&blink_top_bar[0], 2);
        }
        delay_ms(1000);
    }
}