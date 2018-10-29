#include "memory.h"
#include "flags.h"
#include "os/log_macros.h"
#include "peripherals/nonvolatile_memory.h"
#include "relays.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void memory_init(void) {
    nonvolatile_memory_init();
    log_register();
}

/* ************************************************************************** */

// new versions

uint16_t frequencyGroupBoundaries[] = {1,     5500,  10500, 20000,
                                       25000, 30000, 55000};
uint16_t addressSlotsPerGroup[] = {0, 1000, 1500, 2500, 3000, 3400, 4000};
uint16_t addressBoundaries[] = {};

uint32_t map_freq_to_addr(uint16_t frequency) {
    uint8_t group = 0;
    while (frequencyGroupBoundaries[group] < frequency) {
        group++;
    }
    printf("group: %u ", group);
    uint16_t old_min = frequencyGroupBoundaries[group - 1];
    printf("old_min: %u ", old_min);
    uint16_t old_max = frequencyGroupBoundaries[group];
    printf("old_max: %u ", old_max);

    uint16_t new_min = addressSlotsPerGroup[group - 1] + 1;
    printf("new_min: %u ", new_min);
    uint16_t new_max = addressSlotsPerGroup[group];
    printf("new_max: %u\n", new_max);

    uint16_t oldRange = (old_max - old_min);
    uint16_t newRange = (new_max - new_min);

    uint32_t temp = frequency - 1;
    uint32_t address = ((temp * newRange) / oldRange) + new_min;

    return address;
}

/* ************************************************************************** */

/*  Frequency Group definitions

    !0 to 5.5
    5.5 to 10.5
    10.5 to 20
    20 to 25
    25 to 30
    30 to 55
*/

#define FREQ_MIN 1
#define FREQ_MAX 55000

#define FREQ_GROUP_1 5500
#define FREQ_GROUP_2 10500
#define FREQ_GROUP_3 20000
#define FREQ_GROUP_4 25000
#define FREQ_GROUP_5 30000
#define FREQ_GROUP_6 55000

uint16_t frequencyGroups[] = {1, 5500, 10500, 20000, 25000, 30000, 55000};

NVM_address_t map_freq_to_addr(uint16_t frequency, uint16_t new_min,
                               uint16_t new_max) {
    uint32_t temp = 0;
    uint32_t address = 0;
    uint16_t old_min;
    uint16_t old_max;
    uint16_t oldRange = (old_max - old_min);
    uint16_t newRange = (new_max - new_min);

    uint8_t i = 0;
    while (frequency < frequencyGroups[i++])

        temp = frequency - FREQ_MIN;

    address = ((temp * newRange) / oldRange) + new_min;

    return (NVM_address_t)address;
}
// Memory configuration
#define MEMORY_BASE_ADDRESS 46000

NVM_address_t convert_memory_address(uint16_t frequency) {
    NVM_address_t address = 0;

    if (frequency < FREQ_MIN) {
        // wrong
    } else if (frequency < FREQ_GROUP_1) { // 5.5M wide, 1000 slots, 5k/slot
        address = map_freq_to_addr(frequency, 1, 1000);
    } else if (frequency < FREQ_GROUP_2) { // 5M wide, 500 slots, 10k/slot
        address = map_freq_to_addr(frequency, 1001, 1500);
    } else if (frequency < FREQ_GROUP_3) { // 9.5M wide, 1000 slots, 9k/slot
        address = map_freq_to_addr(frequency, 1501, 2500);
    } else if (frequency < FREQ_GROUP_4) { // 5M wide, 500 slots, 10k/slot
        address = map_freq_to_addr(frequency, 2501, 3000);
    } else if (frequency < FREQ_GROUP_5) { // 5M wide, 400 slots, 12k/slot
        address = map_freq_to_addr(frequency, 3001, 3400);
    } else if (frequency < FREQ_GROUP_6) { // 25M wide, 600 slots, 41k/slot
        address = map_freq_to_addr(frequency, 3401, 4000);
    } else if (frequency > FREQ_MAX) {
        // wrong
    }

    address += MEMORY_BASE_ADDRESS;
    address &= ~1;

    return address;
}

/* -------------------------------------------------------------------------- */

void memory_store(NVM_address_t address) {
    // uint8_t i;
    // uint8_t buffer[64];
    // relays_t readRelays;

    // // Make sure we aren't wasting an erase/write cycle
    // readRelays.all = memory_recall(address);
    // if (readRelays.all == currentRelays[system_flags.antenna].all)
    //     return;

    // LOG_INFO({
    //     printf("mem write: %d", address);
    //     print_relays(&currentRelays[system_flags.antenna]);
    // println("");
    // });

    // // Read existing block into buffer
    // flash_read_block(address, buffer);

    // // Mask off everything but bottom 6 bits(64 addresses)
    // i = address & 0x003f;

    // // Pack the caps and hiloz into the low byte, load inds into the high
    // byte buffer[i] = currentRelays[system_flags.antenna].top; buffer[i + 1] =
    // currentRelays[system_flags.antenna].bot;

    // // Write the edited buffer into flash
    // flash_block_erase(address);
    // flash_block_write(address, buffer);
}

uint32_t memory_recall(NVM_address_t address) {
    relays_t readRelays;
    readRelays.all = 0;

    readRelays.top = flash_read_byte(address);
    readRelays.bot = flash_read_byte(address + 1);

    LOG_INFO({
        printf("mem read: %d", address);
        print_relays(&readRelays);
        println("");
    });

    return readRelays.all;
}