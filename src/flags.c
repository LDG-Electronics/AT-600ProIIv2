#include "includes.h"

/* ************************************************************************** */

saved_flags_s        saved_flags;

/* ************************************************************************** */

void retrieve_flags(void)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t d = 0;
    uint8_t valid = 0;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("retrieve_flags");
    #endif
    
    while (i < 250)
    {
        if ((internal_eeprom_read(i) & 0x80) == 0) 
        {
            valid = 1;
            break;
        }
        i += 4;
    }

    if (valid == 1) {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  found valid records");
        #endif
        
        swrThreshIndex = (internal_eeprom_read(i) & 0x07);
        currentRelays.top = internal_eeprom_read(i + 1);
        currentRelays.bot = internal_eeprom_read(i + 2);
        saved_flags.flags = internal_eeprom_read(i + 3);
    } else {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  no valid record");
        #endif
        
        currentRelays.all = 0;
        saved_flags.inBypass = 1;
        saved_flags.forceAllRelays = 1;
        saved_flags.AutoMode = 1;
        swrThreshIndex = 0;
    }
    SWR_threshold_set();
}

void store_flags(void)
{
    uint8_t i = 0;

    uint8_t tempThreshIndex = 0; 
    relays_s tempRelays;
    saved_flags_s temp_flags;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("store_flags");
    #endif
    
    while (i < 250)
    {
        if ((internal_eeprom_read(i) & 0x80) == 0) break;
        i += 4;
    }
    
    tempThreshIndex = (internal_eeprom_read(i) & 0x7f);
    tempRelays.top = internal_eeprom_read(i + 1);
    tempRelays.bot = internal_eeprom_read(i + 2);
    temp_flags.flags = internal_eeprom_read(i + 3);
     
    if ((tempThreshIndex != swrThreshIndex) ||
        (tempRelays.top != currentRelays.top) ||
        (tempRelays.bot != currentRelays.bot) ||
        (temp_flags.flags != saved_flags.flags))
    {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  saving records");
        #endif
        
        internal_eeprom_write(i, 0xff);
        
        i += 4;
        if (i > 248) i = 0;

        internal_eeprom_write(i, (swrThreshIndex & 0x7f));
        internal_eeprom_write(i + 1, currentRelays.top);
        internal_eeprom_write(i + 2, currentRelays.bot);
        internal_eeprom_write(i + 3, saved_flags.flags);
    }
}
