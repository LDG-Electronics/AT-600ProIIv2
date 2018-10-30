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

    printf("%lu\r\n", (uint32_t)map_freq_to_addr(100));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(1000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(2000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(2500));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(3000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(3500));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(6000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(11000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(18000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(21000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(25000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(28000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(35000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(50000));
    printf("%lu\r\n", (uint32_t)map_freq_to_addr(55000));
}

/* ************************************************************************** */
/*  map_freq_to_addr()

    This recreates the behavior of the following mapRange() function,
    automatically filling in the values of a1, a2, b1, and b2.

    float mapRange(float a1, float a2, float b1, float b2, float s) {
        return b1 + (s - a1) * (b2 - b1) / (a2 - a1);
    }

    Group 1(1 to 5.5): 5.5M wide, 1000 slots, 5k/slot
    Group 2(5.5 to 10.5): 5M wide, 500 slots, 10k/slot
    Group 3(10.5 to 20): 9.5M wide, 1000 slots, 9k/slot
    Group 4(20 to 25): 5M wide, 500 slots, 10k/slot
    Group 5(25 to 30): 5M wide, 400 slots, 12k/slot
    Group 6(30 to 55): 25M wide, 600 slots, 41k/slot
*/

uint16_t freqGroupBoundaries[] = {1, 5500, 10500, 20000, 25000, 30000, 55000};
uint16_t addressSlotsPerGroup[] = {0, 1000, 1500, 2500, 3000, 3400, 4000};
// TODO: build test for this
NVM_address_t map_freq_to_addr(uint16_t frequency) {
    // identify frequency group
    uint8_t group = 0;
    while (freqGroupBoundaries[group] < frequency) {
        group++;
    }
    uint16_t inputMin = freqGroupBoundaries[group - 1];
    uint16_t inputMax = freqGroupBoundaries[group];

    uint16_t outputMin = addressSlotsPerGroup[group - 1] + 1;
    uint16_t outputMax = addressSlotsPerGroup[group];

    uint16_t inputRange = (inputMax - inputMin);
    uint16_t outputRange = (outputMax - outputMin);

    return (NVM_address_t)outputMin + (frequency - inputMin) *
                                          (outputMax - outputMin) /
                                          (inputMax - inputMin);
}

// Memory configuration
#define MEMORY_BASE_ADDRESS 55000

NVM_address_t convert_memory_address(uint16_t frequency) {
    NVM_address_t address = map_freq_to_addr(frequency);

    address += MEMORY_BASE_ADDRESS;
    address &= ~1;

    return address;
}

/* ************************************************************************** */
// single memory struct
typedef struct {
    packed_relays_t relayBits; // 2 bytes
    uint16_t frequency;        // 2 bytes
    float bypassSWR;           // 3 bytes
    float finalSWR;            // 3 bytes
    uint8_t recallCount;       // 1 byte
    uint8_t unused;            // 1 byte
} memory_slot_t;               // 12 bytes total

// 12 bytes * 10 = 120 bytes, with 8 left over
#define MEMORIES_PER_FLASH_BLOCK 10

// single block of flash, contains
typedef union {
    struct {
        memory_slot_t memories[MEMORIES_PER_FLASH_BLOCK]; // 120 bytes
        uint8_t unused[8];                                // 8 byte
    };
    uint8_t array[128];
} memory_page_t;

void memory_store(NVM_address_t address) {
    // Make sure we aren't wasting an erase/write cycle
    relays_t existingRelays = read_current_relays();
    relays_t savedRelays;
    savedRelays.all = memory_recall(address);
    if (savedRelays.all == existingRelays.all) {
        return;
    }

    LOG_INFO({
        printf("mem write: %d", address);
        print_relays(&existingRelays);
        println("");
    });

    // compare the two datas bit-by-bit, checking for 0->1 transitions
    bool mustErase = false;
    // for (uint8_t i = 0; i < 8; i++) {
    //     if (!(existingData & (1 << i)) && (newData & (1 << i))) {
    //         mustErase = true;
    //     }
    // }

    // Read existing block into buffer
    uint8_t buffer[FLASH_BUFFER_SIZE];
    flash_read_block(address, buffer);

    uint8_t i = address & FLASH_ELEMENT_MASK;

    // Pack the caps and hiloz into the low byte, load inds into the high byte
    packed_relays_t relayBits = pack_relays(&existingRelays);
    buffer[i] = relayBits.top;
    buffer[i + 1] = relayBits.bot;

    // Write the edited buffer into flash
    flash_block_erase(address);
    flash_block_write(address, buffer);
}

uint32_t memory_recall(NVM_address_t address) {
    relays_t savedRelays;
    savedRelays.all = 0;

    // savedRelays.top = flash_read_byte(address);
    // savedRelays.bot = flash_read_byte(address + 1);

    LOG_INFO({
        printf("mem read: %d", address);
        print_relays(&savedRelays);
        println("");
    });

    return savedRelays.all;
}