#include "tuning.h"
#include "display.h"
#include "flags.h"
#include "os/logging.h"
#include "os/system_time.h"
#include "relays.h"
#include "rf_sensor.h"
#include "tuning_memories.h"
#include "tuning_search.h"
#include "tuning_utils.h"
#include <float.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void tuning_init(void) {
    // register with the logging subsystem
    log_register();

    // init the other tuning files
    tuning_memories_init();
    tuning_search_init();
    tuning_utils_init();
}

/* ************************************************************************** */

tuning_errors_t full_tune(void) {
    LOG_TRACE({ println("full_tune"); });

    system_time_t startTime = get_current_time();

    tuning_errors_t errors = no_errors();
    reset_solution_count();

    // early exit if there's no RF
    if (!wait_for_stable_RF(2500)) {
        errors.noRF = 1;
        return errors;
    }

    measure_RF();
    measure_frequency();
    while (currentRF.frequency == 0) {
        LOG_DEBUG({ println("bad frequency, retrying"); });
        measure_frequency();
    }

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
    if (bestMatch.relays.inds < 3 || bestMatch.relays.caps < 3) {
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

    wait_for_stable_RF(500);
    delay_ms(250);

    measure_RF();
    measure_frequency();
    if (currentRF.frequency == 0) {
        LOG_DEBUG({ println("bad frequency, retrying"); });
        measure_frequency();
        errors.noFreq = 1;
        LOG_WARN({ println("no frequency!"); });
    }
    calculate_watts_and_swr();

    LOG_DEBUG({ printf("frequency: %u KHz\r\n", currentRF.frequency); });
    LOG_INFO({
        printf("tested %u solutions in %lums, ", comparisonCount, time_since(startTime));
        printf("final SWR: %f\r\n", bestMatch.swr);
    });

    // Save the result, if it's good enough
    if (bestMatch.swr < get_SWR_threshold()) {
        uint16_t slot = find_memory_slot(currentRF.frequency);
        LOG_INFO({
            print("saving ");
            print_relays(bestMatch.relays);
            printf(" to slot %u \r\n", slot);
        });
        store_memory(slot, bestMatch.relays);
    } else {
        errors.badMatch = 1;
    }
    return errors;
}

/* -------------------------------------------------------------------------- */

//
#define NUM_OF_MEMORIES 9
#define MAXIMUM_ATTEMPTS 20

tuning_errors_t memory_tune(void) {
    LOG_TRACE({ println("memory_tune"); });

    tuning_errors_t errors = no_errors();
    reset_solution_count();

    // If we fail to find RF, then set an error and exit.
    if (!wait_for_stable_RF(2500)) {
        errors.noRF = 1;
        return errors;
    }

    // Hopefully RF is stable, so refresh our measurements
    measure_RF();
    measure_frequency();
    if (currentRF.frequency == 0) {
        LOG_DEBUG({ println("bad frequency, retrying"); });
        measure_frequency();
        errors.noFreq = 1;
        LOG_WARN({ println("no frequency!"); });
    }

    LOG_DEBUG({ printf("frequency: %u KHz\r\n", currentRF.frequency); });

    // Create and initialize local storage for recalled memories
    relays_t memoryBuffer[NUM_OF_MEMORIES];
    for (uint8_t i = 0; i < NUM_OF_MEMORIES; i++) {
        memoryBuffer[i].all = 0;
    }

    // Use this to count how many memories are recalled
    uint8_t memoriesFound = 0;
    uint16_t slot = find_memory_slot(currentRF.frequency);

    relays_t tempRelays = recall_memory(slot);
    if (tempRelays.all != 0) {
        memoryBuffer[memoriesFound++] = tempRelays;
    }

    uint16_t offset = 0;

    for (uint8_t attempts = 0; attempts < MAXIMUM_ATTEMPTS; attempts++) {
        offset += 1;

        tempRelays = recall_memory(slot + offset);
        if (tempRelays.all != 0) {
            memoryBuffer[memoriesFound++] = tempRelays;
        }

        if (memoriesFound == NUM_OF_MEMORIES) {
            break;
        }

        tempRelays = recall_memory(slot - offset);
        if (tempRelays.all != 0) {
            memoryBuffer[memoriesFound++] = tempRelays;
        }

        if (memoriesFound == NUM_OF_MEMORIES) {
            break;
        }
    }

    // if we didn't successfully recall a memory, set an error and exit
    if (!memoriesFound) {
        LOG_DEBUG({ println("no memories recalled"); });
        errors.noMemory = 1;
        return errors;
    }

    LOG_DEBUG({
        printf("recalled %u memories:\r\n", memoriesFound);
        for (uint8_t i = 0; i < memoriesFound; i++) {
            printf("\t# %u: ", i);
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
    delay_ms(250);
    measure_RF();
    calculate_watts_and_swr();

    // Did we find a valid memory?
    LOG_INFO({ printf("final SWR: %f\r\n", currentRF.swr); });
    if (currentRF.swr < get_SWR_threshold()) {
        LOG_INFO({
            printf("found memory: %f ", currentRF.matchQuality);
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