#include "includes.h"

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

uint16_t map_freq_to_addr(uint16_t frequency, 
                          uint16_t old_min, uint16_t old_max, 
                          uint16_t new_min, uint16_t new_max)
{
    uint32_t temp = 0;
    uint32_t address = 0;
    uint16_t oldRange = (old_max - old_min);
    uint16_t newRange = (new_max - new_min);
    
    temp = frequency - FREQ_MIN;
    
    address = ((temp * newRange) / oldRange) + new_min;
    
    return (uint16_t)address;
}

// Memory configuration
#define MEMORY_BASE_ADDRESS 26000

uint32_t convert_memory_address(uint16_t frequency)
{
    uint32_t address = 0;
    
    if (frequency < FREQ_MIN) {
        // wrong
    } else if (frequency < FREQ_GROUP_1) { // 5.5M wide, 1000 slots, 5k/slot
        address = map_freq_to_addr(frequency, FREQ_MIN, FREQ_GROUP_1, 1, 1000);
        
    } else if (frequency < FREQ_GROUP_2) { // 5M wide, 500 slots, 10k/slot
        address = map_freq_to_addr(frequency, FREQ_GROUP_1, FREQ_GROUP_2, 1000, 1500);
        
    } else if (frequency < FREQ_GROUP_3) { // 9.5M wide, 1000 slots, 9k/slot
        address = map_freq_to_addr(frequency, FREQ_GROUP_2, FREQ_GROUP_3, 1501, 2500);
        
    } else if (frequency < FREQ_GROUP_4) { // 5M wide, 500 slots, 10k/slot
        address = map_freq_to_addr(frequency, FREQ_GROUP_3, FREQ_GROUP_4, 2501, 3000);
        
    } else if (frequency < FREQ_GROUP_5) { // 5M wide, 400 slots, 12k/slot
        address = map_freq_to_addr(frequency, FREQ_GROUP_4, FREQ_GROUP_5, 3001, 3400);
        
    } else if (frequency < FREQ_GROUP_6) { // 25M wide, 600 slots, 41k/slot
        address = map_freq_to_addr(frequency, FREQ_GROUP_5, FREQ_GROUP_6, 3401, 4000);
        
    } else if (frequency > FREQ_MAX) {
        // wrong
    }
    
    address += MEMORY_BASE_ADDRESS; 
    address &= ~1;
    
    return address;
}

/* -------------------------------------------------------------------------- */

//TODO: These functions are disabled in anticipation of overhaul

void memory_store(uint32_t address)
{
    // uint8_t i;
    // uint8_t buffer[64];
    // relays_s readRelays;
    
    // // Make sure we aren't wasting an erase/write cycle [1]
    // readRelays.all = memory_recall(address);
    // readRelays.caps |= 1;
    // if (readRelays.all == currentRelays[system_flags.antenna].all) return;
    // readRelays.caps &= ~1;
    // if (readRelays.all == currentRelays[system_flags.antenna].all) return;
    
    // #if LOG_LEVEL_MEMORY >= LOG_EVENTS
    // print_cat("  mem write: ", address);
    // print_relays_ln(&currentRelays[system_flags.antenna]);
    // #endif
    
    // // Read existing block into buffer
    // flash_block_read(address, buffer);
    
    // // Mask off everything but bottom 6 bits(64 addresses)
    // i = address & 0x003f;
    
    // // Pack the caps and hiloz into the low byte, load inds into the high byte
    // buffer[i] = currentRelays[system_flags.antenna].caps;
    // buffer[i] ^= (-currentRelays[system_flags.antenna].z ^ buffer[i]) & 1;
    // buffer[i + 1] = currentRelays[system_flags.antenna].inds;
    
    // // Write the edited buffer into flash
    // flash_block_erase(address);
    // flash_block_write(address, buffer);
}

uint32_t memory_recall(uint32_t address)
{
    // relays_s readRelays;
    // readRelays.all = 0;
    
    // readRelays.caps = flash_read(address);
    // readRelays.inds = flash_read(address + 1);
    
    // readRelays.z = readRelays.caps & 1;
    // readRelays.caps &= ~1;
    
    // #if LOG_LEVEL_MEMORY >= LOG_INFO
    // print_cat("  mem read: ", address);
    // print_relays_ln(&readRelays);  
    // #endif
    
    // return readRelays.all;
    return 0; // required for function to be disabled
}


/*  Footnote [1]:
    
    This spectacular waste of a CPU cycle is brought to you by SDCC. 
    I wanted to write:
    
    readRelays.all = memory_recall(address);
    if (readRelays.all == currentRelays[system_flags.antenna].all) return;
    readRelays.caps |= 1;
    if (readRelays.all == currentRelays[system_flags.antenna].all) return;
    
    But that causes this output:
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    Found a processor register! WREG
    
    This error is not in the SDCC manual, and it's barely mentioned on the
    internet. The only real discussion I can find ends with this gem:
    
    https://sourceforge.net/p/sdcc/mailman/message/35088893/
    "That's a pic16 issue, so I suggest to ignore it. AFter all the pic16
    port has much bigger issues, such as no longer being able to build the
    regression tests (AFAIK not an SDCC regression, just an incompability
    with more recent gputils)." 
    
*/