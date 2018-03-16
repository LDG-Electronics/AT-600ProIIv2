#include "includes.h"

/* ************************************************************************** */

system_flags_s system_flags;

uint8_t bypassStatus[NUM_OF_ANTENNA_PORTS];

#define FLAG_BLOCK_WIDTH 10

/* ************************************************************************** */

void flags_init(void)
{
    // populate system_flags with default values
    system_flags.ant1Bypass = 1; // default value is bypass
    system_flags.ant2Bypass = 1; // default value is bypass
    system_flags.antenna = 0; // default value is Ant 2
    system_flags.autoMode = 0; // default value is semi mode
    system_flags.peakMode = 0; // default value is NOT peak mode
    system_flags.Scale100W = 0; // default value is 
    swrThreshIndex = 0;

    load_flags();

    // copy stored bypass values to the usable array
    bypassStatus[0] = system_flags.ant1Bypass;
    bypassStatus[1] = system_flags.ant2Bypass;
}

/* -------------------------------------------------------------------------- */

void load_flags(void)
{
    uint8_t address = 0;
    uint8_t valid = 0;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("load_flags");
    #endif
    
    while (address < (FLAG_BLOCK_WIDTH * 20))
    {
        if ((internal_eeprom_read(address) & 0x80) == 0) 
        {
            valid = 1;
            break;
        }
        address += FLAG_BLOCK_WIDTH;
    }

    if (valid == 1) {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  found valid records");
        #endif
        
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
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  no valid record");
        #endif        
    }
    SWR_threshold_set();
}

void save_flags(void)
{
    uint8_t address = 0;
    uint8_t tempThreshIndex = 0; 
    
    relays_s tempRelays[NUM_OF_ANTENNA_PORTS * 2];
    system_flags_s temp_flags;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("save_flags");
    #endif

    while (address < (FLAG_BLOCK_WIDTH * 20))
    {
        if ((internal_eeprom_read(address) & 0x80) == 0) break;
        address += FLAG_BLOCK_WIDTH;
    }
    
    tempThreshIndex = (internal_eeprom_read(address) & 0x7f);
    temp_flags.flags = internal_eeprom_read(address + 1);
    tempRelays[0].top = internal_eeprom_read(address + 2);
    tempRelays[0].bot = internal_eeprom_read(address + 3);
    tempRelays[1].top = internal_eeprom_read(address + 4);
    tempRelays[1].bot = internal_eeprom_read(address + 5);
    tempRelays[0].top = internal_eeprom_read(address + 6);
    tempRelays[0].bot = internal_eeprom_read(address + 7);
    tempRelays[1].top = internal_eeprom_read(address + 8);
    tempRelays[1].bot = internal_eeprom_read(address + 9);
     
    if ((tempThreshIndex != swrThreshIndex) ||
        (tempRelays[0].top != currentRelays[0].top) ||
        (tempRelays[0].bot != currentRelays[0].bot) ||
        (tempRelays[1].top != currentRelays[1].top) ||
        (tempRelays[1].bot != currentRelays[1].bot) ||
        (tempRelays[2].top != preBypassRelays[0].top) ||
        (tempRelays[2].bot != preBypassRelays[0].bot) ||
        (tempRelays[3].top != preBypassRelays[1].top) ||
        (tempRelays[3].bot != preBypassRelays[1].bot) ||
        (temp_flags.flags != system_flags.flags))
    {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  saving records");
        #endif
        
        internal_eeprom_write(address, 0xff);
        
        address += FLAG_BLOCK_WIDTH;
        if (address > (FLAG_BLOCK_WIDTH * 20)) address = 0;

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
