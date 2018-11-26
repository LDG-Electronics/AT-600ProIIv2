#include "memory.h"
#include "flags.h"
#include "os/log_macros.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/relay_driver.h"
#include "relays.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void memory_init(void) {
    nonvolatile_memory_init();
    log_register();
}

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

/*  //! Watch out!
    These frequency numbers are in KHz instead of Hz or MHz!

    If you're used to thinking in MHz(eg one-point-eight megahertz), don't
    forget to mentally divide by 1000.
*/
#define _160M_BOT 1800
#define _160M_TOP 2000
#define _80M_BOT 3500
#define _80M_TOP 4000
#define _40M_BOT 7000
#define _40M_TOP 7300
#define _30M_BOT 10010
#define _30M_TOP 10150
#define _20M_BOT 14000
#define _20M_TOP 14350
#define _17M_BOT 18068
#define _17M_TOP 18168
#define _15M_BOT 21000
#define _15M_TOP 21450
#define _12M_BOT 24890
#define _12M_TOP 24990
#define _10M_BOT 28000
#define _10M_TOP 29700
#define _6M_BOT 50000
#define _6M_TOP 54000

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
    float start = group->start;
    float end = group->end;

    printf("(%f, %f), %u slots", start, end, group->slots);
}

/* -------------------------------------------------------------------------- */

#define NUMBER_OF_GROUPS 21
const frequency_group_t group_edges[NUMBER_OF_GROUPS] = {
    // {Top frequency, Bottom Freq, number of slots}
    {FREQ_MIN, _160M_BOT - 1, 200},     // (0 - 1.8 MHz) 1.8 MHz
    {_160M_BOT, _160M_TOP, 100},        // (1.8 - 2.0 MHz) 0.2 MHz
    {_160M_TOP + 1, _80M_BOT - 1, 200}, // (2.0 - 3.5 MHz) 1.5 MHz
    {_80M_BOT, _80M_TOP, 200},          // (3.5 - 4.0 MHz) 0.5 MHz
    {_80M_TOP + 1, _40M_BOT - 1, 200},  // (4.0 - 7.0 MHz) 3.0 MHz
    {_40M_BOT, _40M_TOP, 100},          // (7.0 - 7.3 MHz) 0.3 MHz
    {_40M_TOP + 1, _30M_BOT - 1, 200},  // (7.3 - 10.1 MHz) 2.8 MHz
    {_30M_BOT, _30M_TOP, 100},          // (10.1 - 10.15 MHz) 0.05 MHz
    {_30M_TOP + 1, _20M_BOT - 1, 200},  // (10.15 - 14.0 MHz) 3.85 MHz
    {_20M_BOT, _20M_TOP, 100},          // (14.0 - 14.35 MHz) 0.35 MHz
    {_20M_TOP + 1, _17M_BOT - 1, 200},  // (14.35 - 18.068 MHz) 3.718 MHz
    {_17M_BOT, _17M_TOP, 100},          // (18.068 - 18.168 MHz) 0.1 MHz
    {_17M_TOP + 1, _15M_BOT - 1, 200},  // (18.168 - 21.0 MHz) 2.832 MHz
    {_15M_BOT, _15M_TOP, 200},          // (21.0 - 21.45 MHz) 0.45 MHz
    {_15M_TOP + 1, _12M_BOT - 1, 200},  // (21.45 - 24.89 MHz) 3.44 MHz
    {_12M_BOT, _12M_TOP, 100},          // (24.89 - 24.99 MHz) 0.1 MHz
    {_12M_TOP + 1, _10M_BOT - 1, 200},  // (24.99 - 28 MHz) 3.01 MHz
    {_10M_BOT, _10M_TOP, 200},          // (28 - 29.7 MHz) 1.7 MHz
    {_10M_TOP + 1, _6M_BOT - 1, 200},   // (29.7 - 50 MHz) 20.3 MHz
    {_6M_BOT, _6M_TOP, 200},            // (50 - 54 MHz) 4 MHz
    {_6M_TOP + 1, FREQ_MAX, 100},       // (54 - 55 MHz) 1 MHz
};

void print_all_frequency_groups(void) {
    uint16_t totalSlots = 0;

    for (uint8_t group = 0; group < NUMBER_OF_GROUPS; group++) {
        totalSlots += group_edges[group].slots;
        printf("group: %u ", group);
        print_frequency_group(&group_edges[group]);
        println("");
    }

    printf("totalSlots: %u\r\n", totalSlots);
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
int32_t map_range(int32_t x, int32_t a1, int32_t a2, int32_t b1, int32_t b2) {
    return b1 + (x - a1) * (b2 - b1) / (a2 - a1);
}

/* ************************************************************************** */

// Memory configuration
// This MUST be at a higher address than the size of the hex file
#define MEMORY_BASE_ADDRESS 80000
#define MEMORY_ANTENNA_OFFSET 4000

NVM_address_t convert_memory_address(uint16_t frequency) {
    LOG_TRACE({ println("convert_memory_address"); });

    if (frequency == UINT16_MAX) {
        LOG_ERROR({ println("invalid frequency"); });
        return 0;
    }

    map_parameters_t map = look_up_map_parameters(frequency);

    NVM_address_t address = (NVM_address_t)map_range(
        frequency, map.inMin, map.inMax, map.outMin, map.outMax);

    //! WARNING
    // Read and chant the incantation before proceeding.
    // Makes address point to two bytes of flash instead of one byte of flash.
    address <<= 1;
    // Set the least significant bit to zero.
    address &= ~1;
    // Pushes address past the address that contains this program.
    address += MEMORY_BASE_ADDRESS;

    if (systemFlags.antenna) {
        address += MEMORY_ANTENNA_OFFSET;
    }

    return address;
}

/* -------------------------------------------------------------------------- */

void address_conversion_test(uint16_t start, uint16_t end, uint16_t step) {
    println("");
    println("-----------------------------------------------");
    printf("(%u,%u) -> (%u)\r\n", start, end, step);

    for (uint16_t freq = start; freq < end; freq += step) {
        printf(" %u -> %lu\r\n", freq, (uint32_t)convert_memory_address(freq));
    }

    println("-----------------------------------------------");
}

/* ************************************************************************** */

relays_t memory_recall(NVM_address_t address) {
    LOG_TRACE({ println("memory_recall"); });
    packed_relays_t relayBits;

    if (address == 0) {
        LOG_ERROR({ println("invalid address"); });
        return read_current_relays();
    }

    relayBits.top = flash_read_byte(address);
    relayBits.bot = flash_read_byte(address + 1);

    LOG_INFO({
        printf("address: %lu", address);
        print_relay_bits(&relayBits);
        println("");
    });

    return unpack_relays(&relayBits);
}

/* -------------------------------------------------------------------------- */

// compare the two variables bit-by-bit, checking for 0->1 transitions
bool must_erase(uint16_t newBits, uint16_t savedBits) {
    for (uint8_t i = 0; i < 16; i++) {
        if (!(savedBits & (1 << i)) && (newBits & (1 << i))) {
            return true;
        }
    }
    return false;
}

void memory_store(NVM_address_t address, relays_t *relays) {
    LOG_TRACE({ println("memory_store"); });

    // return early if we're given a bad address
    if (address == 0) {
        LOG_ERROR({ println("invalid address"); });
        return;
    }

    LOG_INFO({
        printf("address: %lu", address);
        print_relays(relays);
        println("");
    });

    // Grab the current relays
    packed_relays_t relayBits = pack_relays(relays);

    // Pull out what's already at the specified address
    relays_t savedRelays = memory_recall(address);
    packed_relays_t savedRelayBits = pack_relays(&savedRelays);

    // Make sure we aren't wasting an erase/write cycle
    if (savedRelayBits.bits == relayBits.bits) {
        LOG_INFO({ println("Reading existing block"); });
        return;
    }

    // Fill the buffer with the current flash contents
    LOG_INFO({ println("Reading existing block"); });
    uint8_t buffer[FLASH_BUFFER_SIZE];
    flash_read_block(address, buffer);

    // identify which element in the array corresponds to our address
    uint8_t element = address & FLASH_ELEMENT_MASK;

    // update the buffer with our new data
    buffer[element] = relayBits.top;
    buffer[element + 1] = relayBits.bot;

    // erase, but only if necessary
    if (must_erase(relayBits.bits, savedRelayBits.bits)) {
        LOG_INFO({ println("Erasing block"); });
        flash_block_erase(address);
    }

    // finally, we can write our modified block back into flash
    LOG_INFO({ println("Writing new block"); });
    flash_block_write(address, buffer);
}