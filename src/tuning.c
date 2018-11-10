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
static uint8_t LOG_LEVEL = L_TRACE;

/* ************************************************************************** */

const uint8_t tuneStep[] = {0,  1,  2,  4,  6,  9,  12,  16,  21,  27, 34,
                            42, 51, 61, 72, 84, 97, 111, 126, 127, 127};

/* ************************************************************************** */

tuning_flags_t tuning_flags;

#define clear_tuning_flags() tuning_flags.errors = 0

/* -------------------------------------------------------------------------- */

typedef struct {
    relays_t relays;
    float matchQuality;
    float swr;
    float forward;
    uint16_t attemptNumber;
} match_t;

match_t bestMatch;
match_t bypassMatch;

/*

    output:
    "(C22, L11, Z0, A0) Q: 1896.000000, SWR: 1.886656, FWD: 1810.062500, #: 213"
*/
void print_match(match_t *match) {
    print_relays(&match->relays);
    printf(" Q: %f, SWR: %f, FWD: %f, #: %u", match->matchQuality, match->swr,
           match->forward, match->attemptNumber);
}

match_t reset_match(void) {
    match_t match;
    match.relays.all = 0;
    match.matchQuality = FLT_MAX;
    match.swr = FLT_MAX;
    match.forward = 0;
    match.attemptNumber = 0;

    return match;
}

void reset_match_data(match_t *match) {
    match->relays.all = 0;
    match->matchQuality = FLT_MAX;
    match->swr = FLT_MAX;
    match->forward = 0;
    match->attemptNumber = 0;
}

/* ************************************************************************** */

void tuning_init(void) { log_register(); }

/* ************************************************************************** */

typedef union {
    struct {
        uint8_t maxCap;
        uint8_t maxInd;
        uint8_t minCap;
        uint8_t minInd;
    };
    uint32_t all;
} search_area_t;

// Indexes used by coarse_tune()
search_area_t searchArea;

/*  print_search_area() prints the current search_area

    Output: "area: (maxCap , minCap) (maxInd , minInd)"
*/
void print_search_area(search_area_t *area) {
    printf("area: C(%d,%d), L(%d,%d)\r\n", area->maxCap, area->minCap,
           area->maxInd, area->minInd);
}

/* ************************************************************************** */

// File scope: stores the number of solutions tried
uint16_t solutionCount;
uint16_t prevSolutionCount;

void reset_solution_count(void) {
    solutionCount = 0;
    prevSolutionCount = 0;
}

/*  print_solution_count() shows the number of tested tuning solutions

    Output: "solutionCount: iii new: jjj"
*/
void print_solution_count(void) {
    uint16_t difference = solutionCount - prevSolutionCount;

    printf("solutionCount: %d new: %d", solutionCount, difference);

    prevSolutionCount = solutionCount;
}

/* -------------------------------------------------------------------------- */

void save_new_best_match(relays_t *relays) {
    bestMatch.relays = *relays;
    bestMatch.matchQuality = currentRF.matchQuality;
    bestMatch.swr = currentRF.swr;
    bestMatch.forward = currentRF.forward;
    bestMatch.attemptNumber = solutionCount;

    LOG_DEBUG({
        print("saving: ");
        print_match(&bestMatch);
        println("");
    });
}

int8_t test_next_solution(relays_t *relays) {
    solutionCount++;

    if (put_relays(relays) == -1) {
        tuning_flags.relayError = 1;
        return -1;
    }

    if (!wait_for_stable_RF(50)) {
        tuning_flags.lostRF = 1;
        return -1;
    }

    measure_RF();
    calculate_watts_and_swr();

    if (currentRF.matchQuality < bestMatch.matchQuality) {
        save_new_best_match(relays);
        return 1;
    }

    return 0;
}

/* -------------------------------------------------------------------------- */

void L_zip(relays_t *relays, uint8_t caps, uint8_t startingIndex) {
    uint8_t tryIndex = startingIndex;

    relays->caps = caps;
    while (tuneStep[tryIndex] < searchArea.maxInd) {
        relays->inds = tuneStep[tryIndex];
        if (test_next_solution(relays) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

void LC_zip(relays_t *relays) {
    uint8_t tryIndex = 0;

    while (tuneStep[tryIndex] < searchArea.maxCap) {
        relays->caps = tuneStep[tryIndex];
        relays->inds = tuneStep[tryIndex];
        if (test_next_solution(relays) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

match_t test_z(uint8_t z) {
    LOG_TRACE({ println("test_z"); });
    // prepare the relay
    relays_t relays;
    relays.all = 0;
    relays.z = z;

    // draw some lines
    LC_zip(&relays);
    if (tuning_flags.errors != 0) {
        return bestMatch;
    }
    L_zip(&relays, 3, 0);
    if (tuning_flags.errors != 0) {
        return bestMatch;
    }
    L_zip(&relays, 7, 1);
    if (tuning_flags.errors != 0) {
        return bestMatch;
    }

    return bestMatch;
}

match_t restore_best_z(match_t hizMatch, match_t lozMatch) {
    if (hizMatch.matchQuality < lozMatch.matchQuality) {
        return hizMatch;
    } else if (hizMatch.matchQuality == lozMatch.matchQuality) {
        if (hizMatch.forward > lozMatch.forward) {
            return hizMatch;
        } else {
            return lozMatch;
        }
    } else {
        return lozMatch;
    }
}

match_t hiloz_tune(void) {
    LOG_TRACE({ println("hiloz_tune"); });

    reset_match_data(&bestMatch);
    match_t lozMatch = test_z(0);
    if (tuning_flags.errors != 0) {
        return bestMatch;
    }
    reset_match_data(&bestMatch);
    match_t hizMatch = test_z(1);
    if (tuning_flags.errors != 0) {
        return bestMatch;
    }

    return restore_best_z(hizMatch, lozMatch);
}

/* -------------------------------------------------------------------------- */
/*  coarse_tune() searches across the entire set of possible solutions

    It uses two nested loops to cycle through capacitors, move to the next
    inductor, then cycle through capacitors, repeating this pattern across the
    entire solution set.

    It is important to have the inner loop pick capacitors and the outer loop
    pick inductors. The LDG switched L design places the capacitor bank between
    the RF path and ground, while the inductors are in series between the RF
    input and output.
*/
void coarse_tune(void) {
    LOG_TRACE({ println("coarse_tune"); });
    float earlyExitMatchQuality = (bypassMatch.matchQuality / 2);
    relays_t relays;

    uint8_t inductorIndex = 0;
    while (tuneStep[inductorIndex] < searchArea.maxInd) {
        relays.inds = tuneStep[inductorIndex++];

        uint8_t capacitorIndex = 0;
        while (tuneStep[capacitorIndex] < searchArea.maxCap) {
            relays.caps = tuneStep[capacitorIndex++];

            int8_t result = test_next_solution(&relays);
            if (result == -1) {
                LOG_WARN({ println("solution failed!"); });
                return;
            }

            if (bestMatch.matchQuality <= earlyExitMatchQuality) {
                LOG_INFO({ println("early exit!"); });
                goto EXIT;
            }
        }
    }
EXIT:
    LOG_INFO({
        print_solution_count();
        println("");
    });
}

/* -------------------------------------------------------------------------- */

void inductor_sweep(uint8_t width) {
    LOG_TRACE({ println("inductor_sweep"); });

    relays_t relays = bestMatch.relays;

    uint8_t maxInd = searchArea.maxInd;
    if (relays.inds < maxInd - width) {
        maxInd = relays.inds + width;
    }

    uint8_t minInd = 0;
    if (relays.inds > width) {
        minInd = relays.inds - width;
    }

    relays.inds = minInd;
    while (relays.inds < maxInd) {
        relays.inds++;

        int8_t result = test_next_solution(&relays);
        if (result == -1) {
            LOG_WARN({ println("solution failed!"); });
            return;
        }
    }

    LOG_INFO({
        print_solution_count();
        println("");
    });
}

void capacitor_sweep(uint8_t width) {
    LOG_TRACE({ println("capacitor_sweep"); });

    relays_t relays = bestMatch.relays;

    uint8_t maxCap = searchArea.maxCap;
    if (relays.caps < maxCap - width) {
        maxCap = relays.caps + width;
    }

    uint8_t minCap = 0;
    if (relays.caps > width) {
        minCap = relays.caps - width;
    }

    relays.caps = minCap;
    while (relays.caps < maxCap) {
        relays.caps++;

        int8_t result = test_next_solution(&relays);
        if (result == -1) {
            LOG_WARN({ println("solution failed!"); });
            return;
        }
    }
}

/* -------------------------------------------------------------------------- */

/*  reset_search_area() clears search_area to it's default, widest values

    The starting search area is essentially the entire solution space, starting
    at (0,0) and ending at either the full maximum(255 or 127), or by the L or
    C limited value (top two relays disabled).
*/
#define L_LIMIT_FREQUENCY 20000 // 20mhz
#define C_LIMIT_FREQUENCY 30000 // 30mhz
void reset_search_area(search_area_t *area) {
    area->all = 0;
    // minCap and minInd are already set

    // Find maximum C
    if (currentRF.frequency < C_LIMIT_FREQUENCY) {
        area->maxCap = MAX_CAPACITORS;
    } else {
        area->maxCap = (MAX_CAPACITORS >> 2);
    }

    // Find maximum L
    if (currentRF.frequency < L_LIMIT_FREQUENCY) {
        area->maxInd = MAX_INDUCTORS;
    } else {
        area->maxInd = (MAX_INDUCTORS >> 2);
    }

    LOG_INFO({ print_search_area(area); });
}

void full_tune(void) {
    LOG_TRACE({ println("full_tune"); });

    // clean up and reset stuff before we start tuning
    clear_tuning_flags();
    reset_solution_count();
    reset_match_data(&bestMatch);
    reset_match_data(&bypassMatch);
    reset_search_area(&searchArea);

    // make a note of the bypass conditions
    test_next_solution(&bypassRelays);
    bypassMatch = bestMatch;
    reset_match_data(&bestMatch);

    // identify the correct hi/lo z setting
    bestMatch = hiloz_tune();
    if (tuning_flags.errors != 0) {
        return;
    }

    //
    coarse_tune();
    if (tuning_flags.errors != 0) {
        return;
    }

    inductor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }
    capacitor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }

    if (bestMatch.relays.inds < 3) {
        test_z(!bestMatch.relays.z);
        if (tuning_flags.errors != 0) {
            return;
        }
    }

    inductor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }
    capacitor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }
    inductor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }
    capacitor_sweep(10);
    if (tuning_flags.errors != 0) {
        return;
    }

    // re-publish the final results
    if (put_relays(&bestMatch.relays) == -1) {
        tuning_flags.relayError = 1;
        return;
    }

    measure_RF();
    calculate_watts_and_swr();

    // Save the result, if it's good enough
    LOG_INFO({ printf("final SWR: %f\r\n", currentRF.swr); });
    if (currentRF.swr < 1.7) {
        NVM_address_t address = convert_memory_address(currentRF.frequency);
        if (address) {
            memory_store(address, &bestMatch.relays);
        }
        return;
    }
}

/* -------------------------------------------------------------------------- */

// Memory tuning utilities

#define MEMORY_WIDTH 2
#define NUM_OF_MEMORIES 9

float bestMemorySWR;
relays_t bestMemory;
relays_t memoryBuffer[NUM_OF_MEMORIES];

static void prepare_memories(void) {
    LOG_TRACE({ println("prepare_memories"); });

    bestMemorySWR = FLT_MAX;
    bestMemory.all = 0;

    // prepare the address
    NVM_address_t address = convert_memory_address(currentRF.frequency);
    if (address) {
    }
    address -= ((NUM_OF_MEMORIES - 1) / 2);
    uint8_t memoryOffset = 0;

    // Read the memory and its neighbors
    for (uint8_t i = 0; i < NUM_OF_MEMORIES; i++) {
        memoryBuffer[i] = memory_recall(address + memoryOffset);
        memoryOffset += MEMORY_WIDTH;
    }
}

static void test_memory(relays_t *memory) {
    LOG_TRACE({ println("test_memory"); });

    put_relays(memory);
    measure_RF();
    calculate_watts_and_swr();

    LOG_INFO({
        printf("SWR: %f\r\n", currentRF.matchQuality);
        print_relays(memory);
        println("");
    });

    if (currentRF.matchQuality < bestMemorySWR) {
        bestMemorySWR = currentRF.matchQuality;
        bestMemory = *memory;
    }
}

void memory_tune(void) {
    LOG_TRACE({ println("memory_tune"); });

    clear_tuning_flags();

    // If we fail to find RF, then set an error and exit.
    if (!poll_for_RF_until2(200)) {
        tuning_flags.noRF = 1;
        return;
    }

    measure_frequency();
    measure_RF();

    prepare_memories();

    test_memory(&currentRelays[systemFlags.antenna]);

    for (uint8_t i = 0; i < NUM_OF_MEMORIES; i++) {
        test_memory(&memoryBuffer[i]);
        delay_ms(25);
    }

    put_relays(&bestMemory);
    measure_RF();
    calculate_watts_and_swr();

    // Did we find a valid memory?
    if (currentRF.swr < 1.7) {
        LOG_INFO({
            printf("found memory: %f ", currentRF.swr);
            print_relays(&currentRelays[systemFlags.antenna]);
            println("");
        });
        // returning with no flags means success
        return;
    }

    // We must not have found a valid memory
    tuning_flags.noMemory = 1;
    return;
}

/* -------------------------------------------------------------------------- */

/*  Notes on tuning_followup_animation():

    This processes the error flags set during the tuning cycle.

    In theory, tuning should only allow one error to happen before exiting.
    With that in mind, this function will only display one error message.

    This decision was made in part to avoid the awful possibility of everything
    going wrong and then the tuner displaying 8 seconds of random blinks.
*/
void tuning_followup_animation(void) {
    display_clear();
    // delay_ms(1000);

    if (tuning_flags.errors != 0) {
        LOG_ERROR({ print("Error: "); });
        if (tuning_flags.lostRF == 1) {
            LOG_ERROR({ println("lostRF"); });

            repeat_animation(&blink_both_bars[0], 2);

        } else if (tuning_flags.noRF == 1) {
            LOG_ERROR({ println("noRF"); });

            repeat_animation(&blink_both_bars[0], 1);

        } else if (tuning_flags.relayError == 1) {
            LOG_ERROR({ println("relayError"); });

            // relay_error_blink();
        }
    } else {
        LOG_INFO({ printf("bestMatch.swr: %f\r\n", bestMatch.swr); });
        if (bestMatch.swr < 1.7) {
            LOG_INFO({ println("good match"); });

            play_animation(&center_crawl[0]);

        } else if (bestMatch.swr < 3.5) {
            LOG_INFO({ println("decent match"); });

            // led_blink(2, MEDIUM);

        } else if (bestMatch.swr >= 3.5) {
            LOG_INFO({ println("badMatch"); });

            // led_blink(3, MEDIUM);
        }
        // delay_ms(1000);
    }
}