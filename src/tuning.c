#include "includes.h"
#include "tuning.h"
#include "calibration.h"
#include "delay.h"
#include "display.h"
#include "memory.h"

/* ************************************************************************** */

const uint8_t tuneStep[] = {
    0, 1, 2, 4, 6, 9, 12, 16, 21, 27, 34, 42, 51, 61, 72, 84, 97, 111, 126, 142, 159, 177, 194, 210, 225, 239, 252, 255};
    
union {
    struct {
        unsigned C : 1;
        unsigned L : 1;
    };
    uint8_t all;
} freq_limits;

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

/* -------------------------------------------------------------------------- */

tuning_flags_s tuning_flags; 
int status;

// Indexes used by coarse_tune()
search_area_t search_area;
search_index_t max_index;

// Possible tune solutions, to be sent to put_relays()
relays_s nextSolution;

// The best tuning answer at any given time
relays_s bestSolution;
double bestSWR;
uint16_t bestFWD;

// 
double bypassSWR;
uint16_t bypassFWD;

// 
relays_s hizSolution;
double hizSWR;
uint16_t hizFWD;

//
relays_s lozSolution;
double lozSWR;
uint16_t lozFWD;

// 
uint16_t solutionCount;
uint16_t prevSolutionCount;

/* ************************************************************************** */

#define clear_tuning_flags(); tuning_flags.errors = 0; 

/* ************************************************************************** */

// Tuning module debug functions

/*  print_search_area() prints the current search_area
    
    Output is: "area: (maxCap , minCap) (maxInd , minInd)"
*/
void print_search_area(search_area_t *print_area)
{
    uint32_t temp = print_area->all; // satisfy compiler whining
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_str("\t");
    print_cat("area: (", print_area->maxCap);
    print_cat(" , ", print_area->minCap);
    print_cat(") (", print_area->maxInd);
    print_cat(" , ", print_area->minInd);
    print_str_ln(")");
    #endif
}

/*  print_solution_count() shows the number of tested tuning solutions
    
    Output is: "solutionCount: iii new: jjj"
*/
void print_solution_count(void)
{
    uint16_t difference = solutionCount - prevSolutionCount;
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_str("\t");
    print_cat("solutionCount: ", solutionCount);
    print_cat(" new: ", difference);
    print_ln();
    #endif
    
    prevSolutionCount = solutionCount;
}

/* -------------------------------------------------------------------------- */

/*  Frequency Limits:
    
    This prevents damage to the tuner by disabling the largest 2 values of Caps
    or Inds if the current frequency is over a certain threshold.
    
    L Limit is enabled at 20MHz.
    C Limit is enabled at 30MHz.
*/

#define L_LIMIT_FREQUENCY 20000 // 20mhz
#define C_LIMIT_FREQUENCY 30000 // 30mhz

void check_relay_limits(void)
{
    freq_limits.all = 0;

    if (currentRF.frequency > L_LIMIT_FREQUENCY) freq_limits.L = 1;
    if (currentRF.frequency > C_LIMIT_FREQUENCY) freq_limits.C = 1;
}

uint8_t get_l_limit_max(void)
{
    if (freq_limits.L == 1) {
        return (MAX_INDUCTORS >> 2);
    }
    return MAX_INDUCTORS;
}

uint8_t get_c_limit_max(void)
{
    if (freq_limits.C == 1) {
        return (MAX_CAPACITORS >> 2);
    }
    return MAX_CAPACITORS;
}

/* -------------------------------------------------------------------------- */

// Solution-related utility functions
void clear_best_solution(void)
{
    bestSolution.all = 0;
    bestSWR = DBL_MAX;
    bestFWD = 0;
}

void clear_all_solutions(void)
{
    nextSolution.all = 0;
    
    clear_best_solution();
    
    // Clear bypass
    bypassSWR = DBL_MAX;
    bypassFWD = 0;
    
    // Clear hiz
    hizSolution.all = 0;
    hizSWR = DBL_MAX;
    hizFWD = 0;
    
    // Clear loz
    lozSolution.all = 0;
    lozSWR = DBL_MAX;
    lozFWD = 0;
    
    solutionCount = 0;
    prevSolutionCount = 0;
}

/*  reset_search_area() clears search_area to it's default, widest values

    The starting search area is essentially the entire solution space, starting
    at (0,0) and ending at either the full maximum(255 or 127), or by the L or 
    C limited value (top two relays disabled).
*/
void reset_search_area(void)
{
    search_area.all = 0;
    max_index.all = 0;
    check_relay_limits();
    
    // Find maximum C
    search_area.maxCap = get_c_limit_max();
    while (tuneStep[++max_index.caps + 1] < search_area.maxCap);

    // Find maximum L
    search_area.maxInd = get_l_limit_max();
    while (tuneStep[++max_index.inds + 1] < search_area.maxInd);
    
    // Find minimum C
    search_area.minCap = 0;
    
    // Find minimum L
    search_area.minInd = 0;
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_search_area(&search_area);
    #endif
}

/* -------------------------------------------------------------------------- */

/*  test_next_solution()
    
*/
void save_new_best_solution(void)
{
    bestSolution.all = nextSolution.all;
    bestSWR = currentRF.swr;
    bestFWD = currentRF.forward;
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_str("\t\t");
    print_relays(&bestSolution);
    print_current_SWR();
    print_ln();
    #endif
}

int8_t test_next_solution(uint8_t testMode)
{
    solutionCount++;
    
    if (put_relays(&nextSolution) == -1) goto RELAY_ERROR;
    if (SWR_stable_average() != 0) goto LOST_RF;
    
    if (testMode == 0) {
        if (currentRF.swr < bestSWR) {
            save_new_best_solution();
        } else if (currentRF.swr == bestSWR) {
            if (currentRF.forward > bestFWD){
                save_new_best_solution();
            }
        }
    } else if (testMode == 1) {
        if ((currentRF.swr < bestSWR) || (currentRF.forward > bestFWD)) {
            save_new_best_solution();
        }
    }
    return 0;
    
RELAY_ERROR:
    tuning_flags.relayError = 1;
    return(-1);
    
LOST_RF:
    tuning_flags.lostRF = 1;
    return (-1);
}

/* -------------------------------------------------------------------------- */

void L_zip(uint8_t caps, uint8_t startingIndex)
{
    uint8_t tryIndex = startingIndex;
    
    nextSolution.caps = caps;
    while (tryIndex < max_index.inds)
    {
        nextSolution.inds = tuneStep[tryIndex];
        if (test_next_solution(0) == -1) break;
        
        tryIndex += 2;
    }
}

void LC_zip(void)
{
    uint8_t tryIndex = 0;
    
    while (tryIndex < max_index.inds)
    {
        nextSolution.caps = tuneStep[tryIndex];
        nextSolution.inds = tuneStep[tryIndex];
        if (test_next_solution(0) == -1) break;
        
        tryIndex += 2;
    }
}

void test_bypass(void)
{
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str("\t");
    print_str("bypass:");
    print_ln();
    #endif
    
    nextSolution.all = 0;
    test_next_solution(0);

    bypassSWR = bestSWR;
    bypassFWD = bestFWD;
    
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str("\t\t");
    print_relays(&bypassRelays);
    print_catf(" SWR: ", bypassSWR);
    print_cat(" F: ", bypassFWD);
    print_ln();
    #endif
    
    clear_best_solution();
}

void test_loz(void)
{
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str("\t");
    print_str("loz: ")
    print_ln();
    #endif
    
    nextSolution.z = 0;
    LC_zip();
    L_zip(3, 0);
    L_zip(7, 1);
    
    lozSolution.all = bestSolution.all;
    lozSWR = bestSWR;
    lozFWD = bestFWD;
    
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str("\t\t");
    print_relays(&lozSolution);
    print_catf(" SWR: ", lozSWR);
    print_cat(" F: ", lozFWD);
    print_ln();
    #endif
    
    clear_best_solution();
}

void test_hiz(void)
{
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str("\t");
    print_str("hiz: ");
    print_ln();
    #endif
    
    nextSolution.z = 1;
    LC_zip();
    L_zip(3, 0);
    L_zip(7, 1);
    
    hizSolution.all = bestSolution.all;
    hizSWR = bestSWR;
    hizFWD = bestFWD;
    
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str("\t");
    print_relays(&hizSolution);
    print_catf(" SWR: ", hizSWR);
    print_cat(" F: ", hizFWD);
    print_ln();
    #endif
    
    clear_best_solution();
}

void restore_best_z(void)
{
    if (hizSWR < lozSWR) {
        bestSolution.all = hizSolution.all;
        bestSWR = hizSWR;
        bestFWD = hizFWD;
    } else if (hizSWR == lozSWR){
        if (hizFWD > lozFWD) {
            bestSolution.all = hizSolution.all;
            bestSWR = hizSWR;
            bestFWD = hizFWD;
        } else {
            bestSolution.all = lozSolution.all;
            bestSWR = lozSWR;
            bestFWD = lozFWD;
        }
    } else {
        bestSolution.all = lozSolution.all;
        bestSWR = lozSWR;
        bestFWD = lozFWD;
    }
    nextSolution.z = bestSolution.z;
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_str("\t");
    print_str("best z: ");
    print_str("\t");
    print_relays(&bestSolution);
    print_catf(" SWR: ", bestSWR);
    print_cat(" F: ", bestFWD);
    print_ln();
    #endif
}

/*  hiloz_tune() 
    
*/
void hiloz_tune(void)
{
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str_ln("  hiloz_tune");
    #endif
    
    test_bypass();
    test_loz();
    test_hiz();
    
    restore_best_z();
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_solution_count();
    #endif
}

/*  coarse_tune() searches across the entire set of possible solutions
    
    It uses two nested loops to cycle through capacitors, move to the next
    inductor, then cycle through capacitors, repeating this pattern across the
    entire solution set.
    
    It is important to have the inner loop pick capacitors and the outer loop
    pick inductors. The LDG switched L design places the capacitor bank between
    the RF path and ground, while the inductors are in series between the RF
    input and output.
    
    Consquently, the capacitor bank is under less load than the inductor bank,
    and shou
*/
void coarse_tune(void)
{
    search_index_t current_index;
    double earlyExitSWR = (bypassSWR / 2);
    
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str_ln("  coarse_tune");
    #endif
    
    // Do it
    current_index.inds = 0;
    while (current_index.inds <= max_index.inds)
    {
        nextSolution.inds = tuneStep[current_index.inds++];
        
        current_index.caps = 0;
        while (current_index.caps <= max_index.caps)
        {
            nextSolution.caps = tuneStep[current_index.caps++];
            
            if (test_next_solution(0) == -1) return;
            if (bestSWR <= earlyExitSWR)
            {
                #if LOG_LEVEL_TUNING >= LOG_INFO
                print_solution_count();
                #endif
                return;
            }
        }
    }
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_solution_count();
    #endif
}

/*  bracket_tune() searches a narrow subset of the possible solutions
    
    It uses two nested loops to 
*/
void bracket_tune(uint8_t bracket, uint8_t step)
{
    uint16_t tryCap;
    uint16_t tryInd;
    
    search_area_t bracket_area;
    
    double earlyExitSWR = (bestSWR / 2);
    
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str("  bracket_tune");
    print_cat(" (", bracket);
    print_cat(", ", step);
    print_str(") ");
    print_str(" bestSolution:");
    print_relays(&bestSolution);
    #endif
    
    // Define bracket_area
    bracket_area.all = search_area.all;
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
    
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_search_area(&bracket_area);
    #endif
    
    // Do it
    tryInd = bracket_area.minInd;
    while (tryInd < bracket_area.maxInd)
    {
        nextSolution.inds = tryInd;
        
        tryCap = bracket_area.minCap;
        while (tryCap < bracket_area.maxCap)
        {
            nextSolution.caps = tryCap;
            if (test_next_solution(0) == -1) return;
            if (bestSWR < earlyExitSWR)
            {
                #if LOG_LEVEL_TUNING >= LOG_INFO
                print_solution_count();
                #endif
                return;
            }
            tryCap += step;
        }
        tryInd += step;
    }
    #if LOG_LEVEL_TUNING >= LOG_INFO
    print_solution_count();
    #endif
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
void full_tune(void)
{
    uint16_t address = 0;
    
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str_ln("full_tune");
    #endif
    
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
    if (tuning_flags.errors != 0) return;

    coarse_tune();
    bracket_tune(5, 2);
    if (tuning_flags.errors != 0) return;
    
    if (bestSolution.inds < 3) {
        nextSolution.z = ~nextSolution.z;
        
        L_zip(1, 0);
        L_zip(3, 1);
        bracket_tune(2, 1);
        
        nextSolution.z = bestSolution.z;
    }
    
    bracket_tune(30, 4);
    bracket_tune(5, 2);
    if (tuning_flags.errors != 0) return;
    
    bracket_tune(15, 3);
    if (tuning_flags.errors != 0) return;
    
    bracket_tune(5, 2);
    bracket_tune(2, 1);
    if (tuning_flags.errors != 0) return;
    
    // If nothing failed, we can update currentRelays with the best solution
    currentRelays[system_flags.antenna].all = bestSolution.all;
    
    if (put_relays(&currentRelays[system_flags.antenna]) == -1)
    {
        tuning_flags.relayError = 1;
        return;
    }
    
    // Save the result, if it's good enough
    if (bestSWR < SWR1_7)
    {
        #if LOG_LEVEL_TUNING >= LOG_EVENTS
        print_str("  Saving: ");
        print_relays(&currentRelays[system_flags.antenna]);
        print_catf_ln(" with SWR: ", bestSWR);
        #endif
        
        address = convert_memory_address(currentRF.frequency);
        memory_store(address);
        return;
    }
}

/* -------------------------------------------------------------------------- */

// Memory tuning utilities

#define MEMORY_GAP 2
#define NUM_OF_MEMORIES 6

double bestMemorySWR;
relays_s bestMemory;
relays_s memoryBuffer[NUM_OF_MEMORIES];

void prepare_memories(void)
{
    uint16_t address = 0;
    
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str_ln("  prepare_memories");
    #endif
    
    address = convert_memory_address(currentRF.frequency);
    
    bestMemorySWR = DBL_MAX;
    bestMemory.all = 0;
    
    // Read the memory and its neighbors
    memoryBuffer[0].all = memory_recall(address);
    memoryBuffer[1].all = memory_recall(address);
    memoryBuffer[2].all = memory_recall(address - MEMORY_GAP);
    memoryBuffer[3].all = memory_recall(address - MEMORY_GAP);
    memoryBuffer[4].all = memory_recall(address + MEMORY_GAP);
    memoryBuffer[5].all = memory_recall(address + MEMORY_GAP);
    
    // Set the smallest bit of .caps equal to 1
    memoryBuffer[1].caps |= 1;
    memoryBuffer[3].caps |= 1;
    memoryBuffer[5].caps |= 1;
}
    
void test_memory(relays_s* memory)
{
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str_ln("  test_memory");
    #endif
    
    put_relays(memory);
    SWR_stable_average();
    
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_catf(" SWR: ", currentRF.swr);
    print_relays_ln(memory);
    #endif
    
    if (currentRF.swr < bestMemorySWR)
    {
        bestMemorySWR = currentRF.swr;
        bestMemory.all = memory->all;
    }
}

void restore_best_memory(void)
{
    #if LOG_LEVEL_TUNING >= LOG_DETAILS
    print_str_ln("  restore_best_memory");
    #endif
    currentRelays[system_flags.antenna].all = bestMemory.all;
    put_relays(&currentRelays[system_flags.antenna]);
}

/*  memory_tune() tests saved memories for the current frequency, plus neighbors
    
*/
void memory_tune(void)
{
    uint8_t i = 0;
    
    #if LOG_LEVEL_TUNING >= LOG_LABELS
    print_str_ln("memory_tune");
    #endif
    
    clear_tuning_flags();
    
    if (SWR_stable_average() != 0)
    {
        tuning_flags.noRF = 1;
        return;
    }
    
    prepare_memories();
    
    test_memory(&currentRelays[system_flags.antenna]);
    while (i < NUM_OF_MEMORIES)
    {
        test_memory(&memoryBuffer[i]);
        delay_ms(25);
        i++;
    }
    
    restore_best_memory();
    SWR_stable_average();
    
    // Did we find a valid memory?
    if (bestMemorySWR < SWR1_7)
    {
        #if LOG_LEVEL_TUNING >= LOG_EVENTS
        print_catf("  found memory: ", currentRF.swr);
        print_relays_ln(&currentRelays[system_flags.antenna]);
        #endif
        
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
void tuning_followup_animation(void)
{
    display_clear();
    // delay_ms(1000);
    
    if (tuning_flags.errors != 0) {
        #if LOG_LEVEL_TUNING >= LOG_ERROR
        print_str("Error: ");
        #endif
        if (tuning_flags.lostRF == 1) {
            #if LOG_LEVEL_TUNING >= LOG_ERROR
            print_str_ln("lostRF");
            #endif
            
            repeat_animation(&blink_both_bars, 2);
            
        } else if (tuning_flags.noRF == 1) {
            #if LOG_LEVEL_TUNING >= LOG_ERROR
            print_str_ln("noRF");
            #endif
            
            repeat_animation(&blink_both_bars, 1);
            
        } else if (tuning_flags.relayError == 1) {
            #if LOG_LEVEL_TUNING >= LOG_ERROR
            print_str_ln("relayError");
            #endif
            
            // relay_error_blink();
        }
    } else {
        if (bestSWR < SWR1_7) {
            #if LOG_LEVEL_TUNING >= LOG_DETAILS
            print_str_ln("good match");
            #endif
            
            play_animation(&center_crawl);
            
        } else if (bestSWR < SWR3_5) {
            #if LOG_LEVEL_TUNING >= LOG_DETAILS
            print_str_ln("decent match");
            #endif
            
            // led_blink(2, MEDIUM);
            
        } else if (bestSWR >= SWR3_5) {
            #if LOG_LEVEL_TUNING >= LOG_DETAILS
            print_str_ln("badMatch");
            #endif
            
            // led_blink(3, MEDIUM);
        }
        // delay_ms(1000);
    }
}
