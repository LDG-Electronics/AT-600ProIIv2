#include "includes.h"

#include "os/log_macros.h"
#include "peripherals/nonvolatile_memory.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

system_flags_s system_flags;

uint8_t bypassStatus[NUM_OF_ANTENNA_PORTS];

#define FLAG_BLOCK_WIDTH 10
#define NUM_OF_FLAG_BLOCKS 15

/* ************************************************************************** */

void flags_init(void) {
    // populate system_flags with default values
    system_flags.ant1Bypass = 1;  // default value is bypass
    system_flags.ant2Bypass = 1;  // default value is bypass
    system_flags.antenna = 0;     // default value is Ant 2
    system_flags.autoMode = 0;    // default value is semi mode
    system_flags.peakMode = 0;    // default value is NOT peak mode
    system_flags.scaleMode = 0;   // default value is
    system_flags.powerStatus = 1; // default value is 1
    swrThreshIndex = 0;

    // Attempt to load previously saved flags
    load_flags();

    // copy stored bypass values to the usable array
    bypassStatus[0] = system_flags.ant1Bypass;
    bypassStatus[1] = system_flags.ant2Bypass;

    log_register();
}

/* -------------------------------------------------------------------------- */

void load_flags(void) {
    uint8_t address = 0;
    uint8_t valid = 0;

    LOG_TRACE({ println("load_flags"); });

    while (address < (FLAG_BLOCK_WIDTH * NUM_OF_FLAG_BLOCKS)) {
        if ((internal_eeprom_read(address) & 0x80) == 0) {
            valid = 1;
            break;
        }
        address += FLAG_BLOCK_WIDTH;
    }

    if (valid == 1) {
        LOG_INFO({ printf("found valid records at: %d", address); });

        // Read stored values out into their containers
        swrThreshIndex = (internal_eeprom_read(address) & 0x07);
        system_flags.flags = internal_eeprom_read(address + 1);
        currentRelays[0].top = internal_eeprom_read(address + 2);
        currentRelays[0].bot = internal_eeprom_read(address + 3);
        currentRelays[1].top = internal_eeprom_read(address + 4);
        currentRelays[1].bot = internal_eeprom_read(address + 5);
        preBypassRelays[0].top = internal_eeprom_read(address + 6);
        preBypassRelays[0].bot = internal_eeprom_read(address + 7);
        preBypassRelays[1].top = internal_eeprom_read(address + 8);
        preBypassRelays[1].bot = internal_eeprom_read(address + 9);
    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

void save_flags(void) {
    uint8_t address = 0;
    uint8_t tempThreshIndex = 0;

    relays_s tempRelays[NUM_OF_ANTENNA_PORTS * 2];
    system_flags_s temp_flags;

    LOG_TRACE({ println("save_flags"); });

    while (address < (FLAG_BLOCK_WIDTH * NUM_OF_FLAG_BLOCKS)) {
        if ((internal_eeprom_read(address) & 0x80) == 0)
            break;
        address += FLAG_BLOCK_WIDTH;
    }

    tempThreshIndex = (internal_eeprom_read(address) & 0x7f);
    temp_flags.flags = internal_eeprom_read(address + 1);
    tempRelays[0].top = internal_eeprom_read(address + 2);
    tempRelays[0].bot = internal_eeprom_read(address + 3);
    tempRelays[1].top = internal_eeprom_read(address + 4);
    tempRelays[1].bot = internal_eeprom_read(address + 5);
    tempRelays[2].top = internal_eeprom_read(address + 6);
    tempRelays[2].bot = internal_eeprom_read(address + 7);
    tempRelays[3].top = internal_eeprom_read(address + 8);
    tempRelays[3].bot = internal_eeprom_read(address + 9);

    if ((tempThreshIndex != swrThreshIndex) ||
        (temp_flags.flags != system_flags.flags) ||
        (tempRelays[0].top != currentRelays[0].top) ||
        (tempRelays[0].bot != currentRelays[0].bot) ||
        (tempRelays[1].top != currentRelays[1].top) ||
        (tempRelays[1].bot != currentRelays[1].bot) ||
        (tempRelays[2].top != preBypassRelays[0].top) ||
        (tempRelays[2].bot != preBypassRelays[0].bot) ||
        (tempRelays[3].top != preBypassRelays[1].top) ||
        (tempRelays[3].bot != preBypassRelays[1].bot)) {
        internal_eeprom_write(address, 0xff);

        address += FLAG_BLOCK_WIDTH;
        if (address > (FLAG_BLOCK_WIDTH * NUM_OF_FLAG_BLOCKS))
            address = 0;

        LOG_INFO({ printf("saving records at: %d", address); });

        internal_eeprom_write(address, (swrThreshIndex & 0x7f));
        internal_eeprom_write(address + 1, system_flags.flags);
        internal_eeprom_write(address + 2, currentRelays[0].top);
        internal_eeprom_write(address + 3, currentRelays[0].bot);
        internal_eeprom_write(address + 4, currentRelays[1].top);
        internal_eeprom_write(address + 5, currentRelays[1].bot);
        internal_eeprom_write(address + 6, preBypassRelays[0].top);
        internal_eeprom_write(address + 7, preBypassRelays[0].bot);
        internal_eeprom_write(address + 8, preBypassRelays[1].top);
        internal_eeprom_write(address + 9, preBypassRelays[1].bot);
    }
}