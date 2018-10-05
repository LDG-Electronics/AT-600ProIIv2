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

const uint8_t tuneStep[] = {0,   1,   2,   4,   6,   9,   12,  16,  21,  27,
                            34,  42,  51,  61,  72,  84,  97,  111, 126, 142,
                            159, 177, 194, 210, 225, 239, 252, 255};

/* ************************************************************************** */

tuning_flags_s tuning_flags;

#define clear_tuning_flags() tuning_flags.errors = 0

/* -------------------------------------------------------------------------- */

typedef struct {
    relays_s relays;
    double reflectionCoefficient; // reflection coefficient
    double forward;
    uint16_t attemptNumber;
} match_t;

match_t bestMatch;
match_t bypassMatch;
match_t hizMatch;
match_t lozMatch;

relays_s nextSolution;
relays_s bestSolution;
relays_s hizSolution;
relays_s lozSolution;

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

typedef union {
    struct {
        uint8_t caps;
        uint8_t inds;
    };
    uint16_t all;
} search_index_t;

// Indexes used by coarse_tune()
search_area_t search_area;
search_index_t max_index;

/*  print_search_area() prints the current search_area

    Output is: "area: (maxCap , minCap) (maxInd , minInd)"
*/
void print_search_area(search_area_t *print_area) {
    printf("area: C(%d,%d), L(%d,%d)\r\n", print_area->maxCap,
           print_area->minCap, print_area->maxInd, print_area->minInd);
}

/* ************************************************************************** */

// File scope variables storing the number of solutions tried
uint16_t solutionCount;
uint16_t prevSolutionCount;

/*  print_solution_count() shows the number of tested tuning solutions

    Output is: "solutionCount: iii new: jjj"
*/
void print_solution_count(void) {
    uint16_t difference = solutionCount - prevSolutionCount;

    printf("solutionCount: %d new: %d", solutionCount, difference);

    prevSolutionCount = solutionCount;
}

/* -------------------------------------------------------------------------- */

/*  Frequency Limits:

    This prevents damage to the tuner by disabling the largest 2 values of Caps
    or Inds if the current frequency is over a certain threshold.

    L Limit is enabled at 20MHz.
    C Limit is enabled at 30MHz.
*/

union {
    struct {
        unsigned C : 1;
        unsigned L : 1;
    };
    uint8_t all;
} freq_limits;

#define L_LIMIT_FREQUENCY 20000 // 20mhz
#define C_LIMIT_FREQUENCY 30000 // 30mhz

void check_relay_limits(void) {
    freq_limits.all = 0;

    if (currentRF.frequency > L_LIMIT_FREQUENCY) {
        freq_limits.L = 1;
    }
    if (currentRF.frequency > C_LIMIT_FREQUENCY) {
        freq_limits.C = 1;
    }
}

uint8_t get_l_limit_max(void) {
    if (freq_limits.L == 1) {
        return (MAX_INDUCTORS >> 2);
    }
    return MAX_INDUCTORS;
}

uint8_t get_c_limit_max(void) {
    if (freq_limits.C == 1) {
        return (MAX_CAPACITORS >> 2);
    }
    return MAX_CAPACITORS;
}

/* -------------------------------------------------------------------------- */

// Solution-related utility functions
void clear_best_solution(void) {
    bestSolution.all = 0;
    bestMatch.reflectionCoefficient = DBL_MAX;
    bestMatch.forward = 0;
}

void clear_all_solutions(void) {
    nextSolution.all = 0;

    clear_best_solution();

    // Clear bypass
    bypassMatch.reflectionCoefficient = DBL_MAX;
    bypassMatch.forward = 0;

    // Clear hiz
    hizSolution.all = 0;
    hizMatch.reflectionCoefficient = DBL_MAX;
    hizMatch.forward = 0;

    // Clear loz
    lozSolution.all = 0;
    lozMatch.reflectionCoefficient = DBL_MAX;
    lozMatch.forward = 0;

    solutionCount = 0;
    prevSolutionCount = 0;
}

/*  reset_search_area() clears search_area to it's default, widest values

    The starting search area is essentially the entire solution space, starting
    at (0,0) and ending at either the full maximum(255 or 127), or by the L or
    C limited value (top two relays disabled).
*/
void reset_search_area(void) {
    search_area.all = 0;
    max_index.all = 0;
    check_relay_limits();

    // Find maximum C
    search_area.maxCap = get_c_limit_max();
    while (tuneStep[++max_index.caps + 1] < search_area.maxCap) {
        // empty loop
    }

    // Find maximum L
    search_area.maxInd = get_l_limit_max();
    while (tuneStep[++max_index.inds + 1] < search_area.maxInd) {
        // empty loop
    }

    // Find minimum C
    search_area.minCap = 0;

    // Find minimum L
    search_area.minInd = 0;

    LOG_INFO({ print_search_area(&search_area); });
}

/* -------------------------------------------------------------------------- */

void save_new_best_solution(void) {
    bestSolution = nextSolution;
    bestMatch.reflectionCoefficient = currentRF.swr;
    bestMatch.forward = currentRF.forwardADC;

    LOG_INFO({
        print("new best: ");
        print_relays(&bestSolution);
        println("");
    });
}

int8_t test_next_solution(uint8_t testMode) {
    solutionCount++;

    if (put_relays(&nextSolution) == -1) {
        tuning_flags.relayError = 1;
        return (-1);
    }

    SWR_average();
    // if (SWR_stable_average() != 0) {
    //     tuning_flags.lostRF = 1;
    //     return (-1);
    // }

    if (testMode == 0) {
        if (currentRF.swr < bestMatch.reflectionCoefficient) {
            save_new_best_solution();
        } else if (currentRF.swr == bestMatch.reflectionCoefficient) {
            if (currentRF.forwardADC > bestMatch.forward) {
                save_new_best_solution();
            }
        }
    } else if (testMode == 1) {
        if ((currentRF.swr < bestMatch.reflectionCoefficient) ||
            (currentRF.forwardADC > bestMatch.forward)) {
            save_new_best_solution();
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

void L_zip(uint8_t caps, uint8_t startingIndex) {
    uint8_t tryIndex = startingIndex;

    nextSolution.caps = caps;
    while (tryIndex < max_index.inds) {
        nextSolution.inds = tuneStep[tryIndex];
        if (test_next_solution(0) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

void LC_zip(void) {
    uint8_t tryIndex = 0;

    while (tryIndex < max_index.inds) {
        nextSolution.caps = tuneStep[tryIndex];
        nextSolution.inds = tuneStep[tryIndex];
        if (test_next_solution(0) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

void test_bypass(void) {
    LOG_TRACE({ println("bypass:"); });

    nextSolution.all = 0;
    test_next_solution(0);

    bypassMatch = bestMatch;

    LOG_DEBUG({
        print_relays(&bypassRelays);
        printf(" SWR: %f FWD: %d\r\n", bypassMatch.reflectionCoefficient,
               bypassMatch.forward);
    });

    clear_best_solution();
}

void test_loz(void) {
    LOG_TRACE({ println("loz:"); });

    nextSolution.z = 0;
    LC_zip();
    L_zip(3, 0);
    L_zip(7, 1);

    lozSolution = bestSolution;
    lozMatch = bestMatch;

    LOG_DEBUG({
        print_relays(&lozSolution);
        printf(" SWR: %f FWD: %d\r\n", lozMatch.reflectionCoefficient,
               lozMatch.forward);
    });

    clear_best_solution();
}

void test_hiz(void) {
    LOG_TRACE({ println("hiz:"); });

    nextSolution.z = 1;
    LC_zip();
    L_zip(3, 0);
    L_zip(7, 1);

    hizSolution = bestSolution;
    hizMatch = bestMatch;

    LOG_DEBUG({
        print_relays(&hizSolution);
        printf(" SWR: %f FWD: %d\r\n", hizMatch.reflectionCoefficient,
               hizMatch.forward);
    });

    clear_best_solution();
}

void restore_best_z(void) {
    if (hizMatch.reflectionCoefficient < lozMatch.reflectionCoefficient) {
        bestSolution = hizSolution;
        bestMatch = hizMatch;
    } else if (hizMatch.reflectionCoefficient ==
               lozMatch.reflectionCoefficient) {
        if (hizMatch.forward > lozMatch.forward) {
            bestSolution = hizSolution;
            bestMatch = hizMatch;
        } else {
            bestSolution = lozSolution;
            bestMatch = lozMatch;
        }
    } else {
        bestSolution = lozSolution;
        bestMatch = lozMatch;
    }
    nextSolution.z = bestSolution.z;

    LOG_INFO({
        print("best z: ");
        print_relays(&bestSolution);
        printf(" SWR: %f FWD: %d\r\n", bestMatch.reflectionCoefficient,
               bestMatch.forward);
    });
}

void hiloz_tune(void) {
    LOG_TRACE({ println("hiloz_tune:"); });

    test_bypass();
    test_loz();
    test_hiz();

    restore_best_z();

    LOG_INFO({
        print_solution_count();
        println("");
    });
}

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
    LOG_TRACE({ println("coarse_tune:"); });
    search_index_t current_index;
    double earlyExitSWR = (bypassMatch.reflectionCoefficient / 2);

    // Do it
    current_index.inds = 0;
    while (current_index.inds <= max_index.inds) {
        nextSolution.inds = tuneStep[current_index.inds++];

        current_index.caps = 0;
        while (current_index.caps <= max_index.caps) {
            nextSolution.caps = tuneStep[current_index.caps++];

            if (test_next_solution(0) == -1) {
                return;
            }
            if (bestMatch.reflectionCoefficient <= earlyExitSWR) {
                LOG_INFO({
                    print_solution_count();
                    println("");
                });
                return;
            }
        }
    }
    LOG_INFO({
        print_solution_count();
        println("");
    });
}

void bracket_tune(uint8_t bracket, uint8_t step) {
    uint16_t tryCap;
    uint16_t tryInd;

    search_area_t bracket_area;

    double earlyExitSWR = (bestMatch.reflectionCoefficient / 2);

    LOG_TRACE({
        printf("bracket_tune: (%d,%d) bestSolution: ", bracket, step);
        print_relays(&bestSolution);
        println("");
    });

    // Define bracket_area
    bracket_area = search_area;
    if (bestSolution.caps < bracket_area.maxCap - bracket) {
        bracket_area.maxCap = bestSolution.caps + bracket;
    }
    if (bestSolution.inds < bracket_area.maxInd - bracket) {
        bracket_area.maxInd = bestSolution.inds + bracket;
    }
    if (bestSolution.caps > bracket) {
        bracket_area.minCap = bestSolution.caps - bracket;
    }
    if (bestSolution.inds > bracket) {
        bracket_area.minInd = bestSolution.inds - bracket;
    }

    LOG_DEBUG({ print_search_area(&bracket_area); });

    // Do it
    tryInd = bracket_area.minInd;
    while (tryInd < bracket_area.maxInd) {
        nextSolution.inds = tryInd;

        tryCap = bracket_area.minCap;
        while (tryCap < bracket_area.maxCap) {
            nextSolution.caps = tryCap;
            if (test_next_solution(0) == -1) {
                return;
            }
            if (bestMatch.reflectionCoefficient < earlyExitSWR) {
                LOG_INFO({
                    print_solution_count();
                    println("");
                });
                return;
            }
            tryCap += step;
        }
        tryInd += step;
    }
    LOG_INFO({
        print_solution_count();
        println("");
    });
}

/* -------------------------------------------------------------------------- */

/*  full_tune() finds the L and C values that have the lowest SWR

    The full tune has several distinct stages:
    Stage 1: Setup
    Stage 2: Hi/Lo Z
    Stage 3: Coarse Tuning
    Stage 4: Fine Tuning, via several bracket_tune() calls
    Stage 5: Cleanup

    Stage 1: Setup
    The tuning process uses several file-scope variables to track the progress
    through the tuning process. It's vitally important to properly clear the
    values of all of these variables.

    Stage 2: Hi/Lo Z
    This stage _attempts_ to find the correct setting of the HiLoZ relay.
    It's pretty much a cargo cult routine at this point, and only barely works.
    TODO: do science here to find an accurate and reliable process

    Stage 3: Coarse Tune
    This stage does a rough survey of the entire solution area, and ideally it
    will find a solution that is reasonably close to the desired result

    Stage 4: Fine Tuning
    The stage is a series of bracket tunes designed to walk towards the best
    answer, starting from the best answer found during Stage 3

    Stage 5: Cleanup
    This stage involves saving the found result to memory if it's good enough,
    as well as making sure the final answer gets published to the appropriate
    places.

*/
void full_tune(void) {
    uint16_t address = 0;

    LOG_TRACE({ println("full_tune"); });

    clear_tuning_flags();

    // If we fail to find FWD power twice, then set an error and exit.
    if (SWR_stable_average() != 0) {
        if (SWR_stable_average() != 0) {
            tuning_flags.noRF = 1;
            return;
        }
    }

    // Clear out any crap from previous tunes
    clear_all_solutions();
    reset_search_area();

    hiloz_tune();
    if (tuning_flags.errors != 0) {
        return;
    }

    coarse_tune();
    bracket_tune(5, 2);
    if (tuning_flags.errors != 0) {
        return;
    }

    if (bestSolution.inds < 3) {
        nextSolution.z = ~nextSolution.z;

        L_zip(1, 0);
        L_zip(3, 1);
        bracket_tune(2, 1);

        nextSolution.z = bestSolution.z;
    }

    bracket_tune(30, 4);
    bracket_tune(5, 2);
    if (tuning_flags.errors != 0) {
        return;
    }

    bracket_tune(15, 3);
    if (tuning_flags.errors != 0) {
        return;
    }

    bracket_tune(5, 2);
    bracket_tune(2, 1);
    if (tuning_flags.errors != 0) {
        return;
    }

    // If nothing failed, we can update currentRelays with the best solution
    currentRelays[system_flags.antenna] = bestSolution;

    if (put_relays(&currentRelays[system_flags.antenna]) == -1) {
        tuning_flags.relayError = 1;
        return;
    }

    // Save the result, if it's good enough
    if (bestMatch.reflectionCoefficient < SWR1_7) {
        LOG_INFO({
            print("Saving: ");
            print_relays(&currentRelays[system_flags.antenna]);
            printf(" with SWR: %d", bestMatch.reflectionCoefficient);
            println("");
        });

        address = convert_memory_address(currentRF.frequency);
        memory_store(address);
        return;
    }
}

/* -------------------------------------------------------------------------- */

// Memory tuning utilities

#define MEMORY_GAP 2
#define NUM_OF_MEMORIES 6

volatile double bestMemorySWR;
volatile relays_s bestMemory;
relays_s memoryBuffer[NUM_OF_MEMORIES];

void prepare_memories(void) {
    LOG_TRACE({ println("prepare_memories"); });

    uint16_t address = convert_memory_address(currentRF.frequency);

    bestMemorySWR = DBL_MAX;
    bestMemory.all = 0;

    // Read the memory and its neighbors
    memoryBuffer[0].all = memory_recall(address);
    memoryBuffer[1].all = memory_recall(address);
    memoryBuffer[2].all = memory_recall(address - MEMORY_GAP);
    memoryBuffer[3].all = memory_recall(address - MEMORY_GAP);
    memoryBuffer[4].all = memory_recall(address + MEMORY_GAP);
    memoryBuffer[5].all = memory_recall(address + MEMORY_GAP);
}

void test_memory(relays_s *memory) {
    LOG_TRACE({ println("test_memory"); });

    put_relays(memory);
    SWR_stable_average();

    LOG_INFO({
        printf("SWR: %f\r\n", currentRF.swr);
        print_relays_ln(memory);
    });

    if (currentRF.swr < bestMemorySWR) {
        bestMemorySWR = currentRF.swr;
        bestMemory = *memory;
    }
}

void restore_best_memory(void) {
    LOG_TRACE({ println("restore_best_memory"); });

    currentRelays[system_flags.antenna] = bestMemory;
    put_relays(&currentRelays[system_flags.antenna]);
}

void memory_tune(void) {
    LOG_TRACE({ println("memory_tune"); });
    uint8_t i = 0;

    clear_tuning_flags();

    if (SWR_stable_average() != 0) {
        tuning_flags.noRF = 1;
        return;
    }

    prepare_memories();

    test_memory(&currentRelays[system_flags.antenna]);
    while (i < NUM_OF_MEMORIES) {
        test_memory(&memoryBuffer[i]);
        delay_ms(25);
        i++;
    }

    restore_best_memory();
    SWR_stable_average();

    // Did we find a valid memory?
    if (bestMemorySWR < SWR1_7) {
        LOG_INFO({
            printf("found memory: %f", currentRF.swr);
            print_relays_ln(&currentRelays[system_flags.antenna]);
        });

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
        if (bestMatch.reflectionCoefficient < SWR1_7) {
            LOG_INFO({ println("good match"); });

            play_animation(&center_crawl[0]);

        } else if (bestMatch.reflectionCoefficient < SWR3_5) {
            LOG_INFO({ println("decent match"); });

            // led_blink(2, MEDIUM);

        } else if (bestMatch.reflectionCoefficient >= SWR3_5) {
            LOG_INFO({ println("badMatch"); });

            // led_blink(3, MEDIUM);
        }
        // delay_ms(1000);
    }
}