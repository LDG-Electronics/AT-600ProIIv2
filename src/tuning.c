#include "tuning.h"
#include "calibration.h"
#include "display.h"
#include "flags.h"
#include "frequency_counter.h"
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

tuning_flags_s tuning_flags;

#define clear_tuning_flags() tuning_flags.errors = 0

/* -------------------------------------------------------------------------- */

typedef struct {
    relays_t relays;
    float reflectionCoefficient;
    float forward;
    uint16_t attemptNumber;
} match_t;

match_t bestMatch;
match_t bypassMatch;

void reset_match_data(match_t *match) {
    match->relays.all = 0;
    match->reflectionCoefficient = DBL_MAX;
    match->forward = 0;
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
search_area_t search_area;

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

void save_new_best_solution(relays_t *relays) {
    bestMatch.relays = *relays;
    bestMatch.reflectionCoefficient = currentRF.swr;
    bestMatch.forward = currentRF.forward.value;

    LOG_INFO({
        print("new best: ");
        print_relays(&bestMatch.relays);
        println("");
    });
}

int8_t test_next_solution(relays_t *relays) {
    solutionCount++;

    if (put_relays(relays) == -1) {
        tuning_flags.relayError = 1;
        return (-1);
    }

    // If we fail to find FWD power twice, then set an error and exit.
    if (!check_for_RF()) {
        delay_ms(25);
        if (!check_for_RF()) {
            tuning_flags.noRF = 1;
            return -1;
        }
    }
    measure_RF();

    if (currentRF.swr < bestMatch.reflectionCoefficient) {
        save_new_best_solution(relays);
    } else if (currentRF.swr == bestMatch.reflectionCoefficient) {
        if (currentRF.forward.value > bestMatch.forward) {
            save_new_best_solution(relays);
        }
    }
    return 0;
}

/* -------------------------------------------------------------------------- */

void L_zip(relays_t *relays, uint8_t caps, uint8_t startingIndex) {
    uint8_t tryIndex = startingIndex;

    relays->caps = caps;
    while (tuneStep[tryIndex] < search_area.maxInd) {
        relays->inds = tuneStep[tryIndex];
        if (test_next_solution(relays) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

void LC_zip(relays_t *relays) {
    uint8_t tryIndex = 0;

    while (tuneStep[tryIndex] < search_area.maxCap) {
        relays->caps = tuneStep[tryIndex];
        relays->inds = tuneStep[tryIndex];
        if (test_next_solution(relays) == -1) {
            break;
        }

        tryIndex += 2;
    }
}

match_t test_bypass(void) {
    LOG_TRACE({ println("test_bypass:"); });
    reset_match_data(&bestMatch);

    test_next_solution(&bypassRelays);

    LOG_DEBUG({
        print_relays(&bypassRelays);
        printf(" SWR: %f FWD: %d\r\n", bypassMatch.reflectionCoefficient,
               bypassMatch.forward);
    });

    return bestMatch;
}

match_t test_loz(void) {
    LOG_TRACE({ println("test_loz:"); });
    reset_match_data(&bestMatch);

    relays_t relays;
    relays.z = 0;
    LC_zip(&relays);
    L_zip(&relays, 3, 0);
    L_zip(&relays, 7, 1);

    LOG_DEBUG({
        print_relays(&bestMatch.relays);
        printf(" SWR: %f FWD: %d\r\n", bestMatch.reflectionCoefficient,
               bestMatch.forward);
    });

    return bestMatch;
}

match_t test_hiz(void) {
    LOG_TRACE({ println("test_hiz:"); });
    reset_match_data(&bestMatch);

    relays_t relays;
    relays.z = 1;
    LC_zip(&relays);
    L_zip(&relays, 3, 0);
    L_zip(&relays, 7, 1);

    LOG_DEBUG({
        print_relays(&bestMatch.relays);
        printf(" SWR: %f FWD: %d\r\n", bestMatch.reflectionCoefficient,
               bestMatch.forward);
    });

    return bestMatch;
}

void restore_best_z(match_t hizMatch, match_t lozMatch) {
    if (hizMatch.reflectionCoefficient < lozMatch.reflectionCoefficient) {
        bestMatch = hizMatch;
    } else if (hizMatch.reflectionCoefficient ==
               lozMatch.reflectionCoefficient) {
        if (hizMatch.forward > lozMatch.forward) {
            bestMatch = hizMatch;
        } else {
            bestMatch = lozMatch;
        }
    } else {
        bestMatch = lozMatch;
    }

    LOG_INFO({
        print("best z: ");
        print_relays(&bestMatch.relays);
        printf(" SWR: %f FWD: %d\r\n", bestMatch.reflectionCoefficient,
               bestMatch.forward);
    });
}

void hiloz_tune(void) {
    LOG_TRACE({ println("hiloz_tune:"); });

    test_bypass();
    match_t lozMatch = test_loz();
    match_t hizMatch = test_hiz();

    restore_best_z(hizMatch, lozMatch);

    LOG_INFO({
        print_solution_count();
        println("");
    });
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
    LOG_TRACE({ println("coarse_tune:"); });
    // float earlyExitSWR = (bypassMatch.reflectionCoefficient / 2);
    relays_t relays;

    uint8_t inds = 0;
    while (tuneStep[inds] < search_area.maxInd) {
        relays.inds = tuneStep[inds++];

        uint8_t caps = 0;
        while (tuneStep[caps] < search_area.maxCap) {
            relays.caps = tuneStep[caps++];

            if (test_next_solution(&relays) == -1) {
                return;
            }
            // if (bestMatch.reflectionCoefficient <= earlyExitSWR) {
            //     LOG_INFO({
            //         print_solution_count();
            //         println("");
            //     });
            //     return;
            // }
        }
    }
    LOG_INFO({
        print_solution_count();
        println("");
    });
}

/* -------------------------------------------------------------------------- */

search_area_t define_bracket_area(uint8_t bracket) {
    search_area_t bracket_area = search_area;
    if (bestMatch.relays.caps < bracket_area.maxCap - bracket) {
        bracket_area.maxCap = bestMatch.relays.caps + bracket;
    }
    if (bestMatch.relays.inds < bracket_area.maxInd - bracket) {
        bracket_area.maxInd = bestMatch.relays.inds + bracket;
    }
    if (bestMatch.relays.caps > bracket) {
        bracket_area.minCap = bestMatch.relays.caps - bracket;
    }
    if (bestMatch.relays.inds > bracket) {
        bracket_area.minInd = bestMatch.relays.inds - bracket;
    }

    return bracket_area;
}

void bracket_tune(uint8_t bracket, uint8_t step) {
    LOG_TRACE({
        printf("bracket_tune: (%d,%d) bestMatch.relays: ", bracket, step);
        print_relays(&bestMatch.relays);
        println("");
    });
    relays_t relays;

    uint16_t tryCap;
    uint16_t tryInd;

    search_area_t bracket_area = define_bracket_area(bracket);
    LOG_DEBUG({ print_search_area(&bracket_area); });

    float earlyExitSWR = (bestMatch.reflectionCoefficient / 2);

    // Do it
    tryInd = bracket_area.minInd;
    while (tryInd < bracket_area.maxInd) {
        relays.inds = tryInd;

        tryCap = bracket_area.minCap;
        while (tryCap < bracket_area.maxCap) {
            relays.caps = tryCap;
            if (test_next_solution(&relays) == -1) {
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

void vertical_scan(void) {
    uint8_t tryIndex = 0;
    relays_t relays;

    relays.caps = bestMatch.relays.caps;
    while (tuneStep[tryIndex] < search_area.maxInd) {
        relays.inds = tuneStep[tryIndex];
        if (test_next_solution(&relays) == -1) {
            break;
        }

        tryIndex++;
    }
}

void horizontal_scan(void) {
    uint8_t tryIndex = 0;
    relays_t relays;

    relays.inds = bestMatch.relays.inds;
    while (tuneStep[tryIndex] < search_area.maxCap) {
        relays.caps = tuneStep[tryIndex];
        if (test_next_solution(&relays) == -1) {
            break;
        }

        tryIndex++;
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
void reset_search_area(void) {
    search_area.all = 0;

    // Find maximum C
    if (currentRF.frequency < C_LIMIT_FREQUENCY) {
        search_area.maxCap = MAX_CAPACITORS;
    } else {
        search_area.maxCap = (MAX_CAPACITORS >> 2);
    }

    // Find maximum L
    if (currentRF.frequency < L_LIMIT_FREQUENCY) {
        search_area.maxInd = MAX_INDUCTORS;
    } else {
        search_area.maxInd = (MAX_INDUCTORS >> 2);
    }

    // Find minimum C
    search_area.minCap = 0;

    // Find minimum L
    search_area.minInd = 0;

    LOG_INFO({ print_search_area(&search_area); });
}

void reset_tuning_data(void) {
    reset_match_data(&bestMatch);
    reset_match_data(&bypassMatch);

    solutionCount = 0;
    prevSolutionCount = 0;

    reset_search_area();
}

void full_tune(void) {
    LOG_TRACE({ println("full_tune"); });

    clear_tuning_flags();

    // If we fail to find FWD power twice, then set an error and exit.
    if (!check_for_RF()) {
        delay_ms(25);
        if (!check_for_RF()) {
            tuning_flags.noRF = 1;
            return;
        }
    }

    reset_tuning_data();

    hiloz_tune();
    if (tuning_flags.errors != 0) {
        return;
    }

    coarse_tune();
    bracket_tune(5, 2);
    if (tuning_flags.errors != 0) {
        return;
    }

    if (bestMatch.relays.inds < 3) {
        relays_t relays = bestMatch.relays;
        relays.z = !relays.z;
        L_zip(&relays, 1, 0);
        L_zip(&relays, 3, 1);
        bracket_tune(2, 1);
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

    if (put_relays(&bestMatch.relays) == -1) {
        tuning_flags.relayError = 1;
        return;
    }

    // Save the result, if it's good enough
    if (bestMatch.reflectionCoefficient < SWR1_7) {
        LOG_INFO({
            print("Saving: ");
            print_relays(&bestMatch.relays);
            printf(" with SWR: %d", bestMatch.reflectionCoefficient);
            println("");
        });
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

    bestMemorySWR = DBL_MAX;
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

    LOG_INFO({
        printf("SWR: %f\r\n", currentRF.swr);
        print_relays(memory);
        println("");
    });

    if (currentRF.swr < bestMemorySWR) {
        bestMemorySWR = currentRF.swr;
        bestMemory = *memory;
    }
}

void memory_tune(void) {
    LOG_TRACE({ println("memory_tune"); });

    clear_tuning_flags();

    if (!check_for_RF()) {
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

    // Did we find a valid memory?
    if (currentRF.swr < SWR1_7) {
        LOG_INFO({
            printf("found memory: %f", currentRF.swr);
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