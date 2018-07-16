#include "includes.h"
#define LOG_LEVEL logLevel
static uint8_t logLevel = L_SILENT;

/* ************************************************************************** */

void memory_init(void) {
    log_register();
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

uint24_t map_freq_to_addr(uint16_t frequency, uint16_t old_min,
                          uint16_t old_max, uint16_t new_min,
                          uint16_t new_max) {
    uint32_t temp = 0;
    uint32_t address = 0;
    uint16_t oldRange = (old_max - old_min);
    uint16_t newRange = (new_max - new_min);

    temp = frequency - FREQ_MIN;

    address = ((temp * newRange) / oldRange) + new_min;

    return (uint24_t)address;
}

// Memory configuration
#define MEMORY_BASE_ADDRESS 40000

uint24_t convert_memory_address(uint16_t frequency) {
    uint24_t address = 0;

    if (frequency < FREQ_MIN) {
        // wrong
    } else if (frequency < FREQ_GROUP_1) { // 5.5M wide, 1000 slots, 5k/slot
        address = map_freq_to_addr(frequency, FREQ_MIN, FREQ_GROUP_1, 1, 1000);

    } else if (frequency < FREQ_GROUP_2) { // 5M wide, 500 slots, 10k/slot
        address =
            map_freq_to_addr(frequency, FREQ_GROUP_1, FREQ_GROUP_2, 1000, 1500);

    } else if (frequency < FREQ_GROUP_3) { // 9.5M wide, 1000 slots, 9k/slot
        address =
            map_freq_to_addr(frequency, FREQ_GROUP_2, FREQ_GROUP_3, 1501, 2500);

    } else if (frequency < FREQ_GROUP_4) { // 5M wide, 500 slots, 10k/slot
        address =
            map_freq_to_addr(frequency, FREQ_GROUP_3, FREQ_GROUP_4, 2501, 3000);

    } else if (frequency < FREQ_GROUP_5) { // 5M wide, 400 slots, 12k/slot
        address =
            map_freq_to_addr(frequency, FREQ_GROUP_4, FREQ_GROUP_5, 3001, 3400);

    } else if (frequency < FREQ_GROUP_6) { // 25M wide, 600 slots, 41k/slot
        address =
            map_freq_to_addr(frequency, FREQ_GROUP_5, FREQ_GROUP_6, 3401, 4000);

    } else if (frequency > FREQ_MAX) {
        // wrong
    }

    address += MEMORY_BASE_ADDRESS;
    address &= ~1;

    return address;
}

/* -------------------------------------------------------------------------- */

void memory_store(uint24_t address) {
    uint8_t i;
    uint8_t buffer[64];
    relays_s readRelays;

    // Make sure we aren't wasting an erase/write cycle
    readRelays.all = memory_recall(address);
    if (readRelays.all == currentRelays[system_flags.antenna].all)
        return;

    log_info(printf("mem write: %d", address);
             print_relays_ln(&currentRelays[system_flags.antenna]););

    // Read existing block into buffer
    flash_block_read(address, buffer);

    // Mask off everything but bottom 6 bits(64 addresses)
    i = address & 0x003f;

    // Pack the caps and hiloz into the low byte, load inds into the high byte
    buffer[i] = currentRelays[system_flags.antenna].top;
    buffer[i + 1] = currentRelays[system_flags.antenna].bot;

    // Write the edited buffer into flash
    flash_block_erase(address);
    flash_block_write(address, buffer);
}

uint32_t memory_recall(uint24_t address) {
    relays_s readRelays;
    readRelays.all = 0;

    readRelays.top = flash_read(address);
    readRelays.bot = flash_read(address + 1);

    log_info(printf("mem read: %d", address); print_relays_ln(&readRelays););

    return readRelays.all;
}