#include "tuning_memories.h"
#include "nvm_table.h"
#include "os/logging.h"
#include "relay_driver.h"
#include "relays.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
/*  Frequency Group definitions

    These definitions contains the tops and bottoms of all the Ham Bands.

    An up-to-date version of this information should be available at:
    http://www.arrl.org/band-plan

    160 Meters (1.8-2.0 MHz)
    80 Meters (3.5-4.0 MHz)
    60 Meters (5 MHz channels)
    40 Meters (7.0-7.3 MHz)
    30 Meters (10.1-10.15 MHz)
    20 Meters (14.0-14.35 MHz)
    17 Meters (18.068-18.168 MHz)
    15 Meters (21.0-21.45 MHz)
    12 Meters (24.89-24.99 MHz)
    10 Meters (28-29.7 MHz)
    6 Meters (50-54 MHz)

    ** 60 Meters only contains 5 channels and isn't worth dedicating a group to.
*/

/*  OVERLAP_MARGIN is used to enlarge the band boundarys, just to leave some
    wiggle room.
*/
#define OVERLAP_MARGIN 200

/*  //! Watch out!
    These frequency numbers are in KHz instead of Hz or MHz!

    If you're used to thinking in MHz(eg one-point-eight megahertz), don't
    forget to mentally divide by 1000.
*/
#define _160M_BOT 1800 - OVERLAP_MARGIN
#define _160M_TOP 2000 + OVERLAP_MARGIN
#define _80M_BOT 3500 - OVERLAP_MARGIN
#define _80M_TOP 4000 + OVERLAP_MARGIN
#define _40M_BOT 7000 - OVERLAP_MARGIN
#define _40M_TOP 7300 + OVERLAP_MARGIN
#define _30M_BOT 10010 - OVERLAP_MARGIN
#define _30M_TOP 10150 + OVERLAP_MARGIN
#define _20M_BOT 14000 - OVERLAP_MARGIN
#define _20M_TOP 14350 + OVERLAP_MARGIN
#define _17M_BOT 18068 - OVERLAP_MARGIN
#define _17M_TOP 18168 + OVERLAP_MARGIN
#define _15M_BOT 21000 - OVERLAP_MARGIN
#define _15M_TOP 21450 + OVERLAP_MARGIN
#define _12M_BOT 24890 - OVERLAP_MARGIN
#define _12M_TOP 24990 + OVERLAP_MARGIN
#define _10M_BOT 28000 - OVERLAP_MARGIN
#define _10M_TOP 29700 + OVERLAP_MARGIN
#define _6M_BOT 50000 - OVERLAP_MARGIN
#define _6M_TOP 54000 + OVERLAP_MARGIN

#define FREQ_MIN 1
#define FREQ_MAX 55000

/* -------------------------------------------------------------------------- */
/*  Each frequency_group_t object contains that groups start frequency, end
    frequency, and the number of slots that in the memory map that should be
    allocated to that group.
*/
typedef struct {
    uint16_t start;
    uint16_t end;
    uint8_t slots;
} frequency_group_t;

// output: "(1800, 2000), 100 slots"
void print_frequency_group(const frequency_group_t *group) {
    printf("(%05u, %05u), %u slots", group->start, group->end, group->slots);
}

/* -------------------------------------------------------------------------- */

// This big table is used to define the boundaries between bands
#define NUMBER_OF_GROUPS 21
const frequency_group_t group_edges[NUMBER_OF_GROUPS] = {
    // {Top frequency, Bottom Freq, number of slots}
    {FREQ_MIN, _160M_BOT, 200},  // (0 - 1.8 MHz) 1.8 MHz
    {_160M_BOT, _160M_TOP, 100}, // (1.8 - 2.0 MHz) 0.2 MHz
    {_160M_TOP, _80M_BOT, 200},  // (2.0 - 3.5 MHz) 1.5 MHz
    {_80M_BOT, _80M_TOP, 200},   // (3.5 - 4.0 MHz) 0.5 MHz
    {_80M_TOP, _40M_BOT, 200},   // (4.0 - 7.0 MHz) 3.0 MHz
    {_40M_BOT, _40M_TOP, 100},   // (7.0 - 7.3 MHz) 0.3 MHz
    {_40M_TOP, _30M_BOT, 200},   // (7.3 - 10.1 MHz) 2.8 MHz
    {_30M_BOT, _30M_TOP, 100},   // (10.1 - 10.15 MHz) 0.05 MHz
    {_30M_TOP, _20M_BOT, 200},   // (10.15 - 14.0 MHz) 3.85 MHz
    {_20M_BOT, _20M_TOP, 100},   // (14.0 - 14.35 MHz) 0.35 MHz
    {_20M_TOP, _17M_BOT, 200},   // (14.35 - 18.068 MHz) 3.718 MHz
    {_17M_BOT, _17M_TOP, 100},   // (18.068 - 18.168 MHz) 0.1 MHz
    {_17M_TOP, _15M_BOT, 200},   // (18.168 - 21.0 MHz) 2.832 MHz
    {_15M_BOT, _15M_TOP, 200},   // (21.0 - 21.45 MHz) 0.45 MHz
    {_15M_TOP, _12M_BOT, 200},   // (21.45 - 24.89 MHz) 3.44 MHz
    {_12M_BOT, _12M_TOP, 100},   // (24.89 - 24.99 MHz) 0.1 MHz
    {_12M_TOP, _10M_BOT, 200},   // (24.99 - 28 MHz) 3.01 MHz
    {_10M_BOT, _10M_TOP, 200},   // (28 - 29.7 MHz) 1.7 MHz
    {_10M_TOP, _6M_BOT, 200},    // (29.7 - 50 MHz) 20.3 MHz
    {_6M_BOT, _6M_TOP, 200},     // (50 - 54 MHz) 4 MHz
    {_6M_TOP, FREQ_MAX, 100},    // (54 - 55 MHz) 1 MHz
};

void print_all_frequency_groups(void) {
    uint16_t totalSlots = 0;

    println("---------------------------------------------------------------");
    println("## | (start freq -> end freq) | width     | slots | width/slot");
    println("---|--------------------------|-----------|-------|------------");

    for (uint8_t group = 0; group < NUMBER_OF_GROUPS; group++) {
        totalSlots += group_edges[group].slots;
        frequency_group_t tempGroup = group_edges[group];
        uint16_t groupWidth = tempGroup.end - tempGroup.start;

        printf("%02u", group);
        printf(" | (%05u KHz -> %05u KHz)", tempGroup.start, tempGroup.end);
        printf(" | %05u KHz", groupWidth);
        printf(" |  %u ", tempGroup.slots);
        printf(" | %2u KHz/slot", (groupWidth / tempGroup.slots));
        println("");
    }

    println("---------------------------------------------------------------");
    printf("totalSlots: %u\r\n", totalSlots);
    println("---------------------------------------------------------------");
}

/* ************************************************************************** */

typedef struct {
    uint16_t inMin;
    uint16_t inMax;
    uint16_t outMin;
    uint16_t outMax;
} map_parameters_t;

void print_map_parameters(map_parameters_t *map) {
    printf("(%u,%u) -> (%u,%u)", map->inMin, map->inMax, map->outMin,
           map->outMax);
}

map_parameters_t look_up_map_parameters(uint16_t frequency) {
    map_parameters_t map;
    map.outMin = 0;

    for (uint8_t group = 0; group < NUMBER_OF_GROUPS; group++) {
        if (group_edges[group].end > frequency) {
            map.inMin = group_edges[group].start;
            map.inMax = group_edges[group].end;
            map.outMax = map.outMin + group_edges[group + 1].slots;
            break;
        }
        map.outMin += group_edges[group].slots;
    }

    return map;
}

// proportionally translates a number from range a to range b
uint16_t map_range(int32_t x, int32_t a1, int32_t a2, int32_t b1, int32_t b2) {
    return (uint16_t)(b1 + (x - a1) * (b2 - b1) / (a2 - a1));
}

/* ************************************************************************** */

// convert a frequency into a valid memory slot
uint16_t find_memory_slot(uint16_t frequency) {
    LOG_TRACE({ println("find_memory_slot"); });

    LOG_DEBUG({ printf("frequency: %u KHz\r\n", frequency); });
    if (frequency == UINT16_MAX || frequency == 0) {
        LOG_ERROR({ println("invalid frequency"); });
        return 0;
    }

    map_parameters_t map = look_up_map_parameters(frequency);

    uint16_t slot =
        map_range(frequency, map.inMin, map.inMax, map.outMin, map.outMax);

    LOG_DEBUG({ printf("slot: %u\r\n", slot); });

    return slot;
}

/* -------------------------------------------------------------------------- */

void address_conversion_test(uint16_t start, uint16_t end, uint16_t step) {
    println("");
    println("-----------------------------------------------");
    printf("(%u,%u) -> (%u)\r\n", start, end, step);

    for (uint16_t freq = start; freq < end; freq += step) {
        printf(" %u -> %u\r\n", freq, find_memory_slot(freq));
    }

    println("-----------------------------------------------");
}

/* ************************************************************************** */

void tuning_memories_init(void) {

    // initialize the persistent data structures used to store memories
    nvm_table_init();

    //
    log_register();
}

/* ************************************************************************** */

// Recall
relays_t recall_memory(uint16_t slot) {
    // get the raw memory
    table_entry_t memory = nvm_table_read(slot);

    // unpack the memory contents
    relay_bits_t relayBits;
    relayBits.bot = memory.contents[0];
    relayBits.top = memory.contents[1];
    relays_t relays = unpack_relays(relayBits);

    LOG_DEBUG({
        print_relays(relays);
        printf(" recalled from: %u", slot);
        println("");
    });
    return relays;
}

/* -------------------------------------------------------------------------- */

// Store
void store_memory(uint16_t slot, relays_t relays) {
    LOG_DEBUG({
        print_relays(relays);
        printf(" stored at: %u", slot);
        println("");
    });

    // pack the memory
    relay_bits_t relayBits = pack_relays(relays);
    table_entry_t memory = new_table_entry();
    memory.contents[0] = relayBits.bot;
    memory.contents[1] = relayBits.top;

    // write the memory to the array
    nvm_table_write(slot, memory);
}