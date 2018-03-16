#include "includes.h"

/* ************************************************************************** */

system_flags_s system_flags;

bit currentAntenna;

/* ************************************************************************** */

void load_flags(void)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t d = 0;
    uint8_t valid = 0;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("load_flags");
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
        currentRelays[currentAntenna].top = internal_eeprom_read(i + 1);
        currentRelays[currentAntenna].bot = internal_eeprom_read(i + 2);
        system_flags.flags = internal_eeprom_read(i + 3);
    } else {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  no valid record");
        #endif
        
        currentRelays[currentAntenna].all = 0;
        system_flags.inBypass = 1;
        system_flags.AutoMode = 1;
        swrThreshIndex = 0;
    }
    SWR_threshold_set();
}

void save_flags(void)
{
    uint8_t i = 0;

    uint8_t tempThreshIndex = 0; 
    relays_s tempRelays;
    system_flags_s temp_flags;
    
    #if LOG_LEVEL_FLAGS >= LOG_LABELS
    print_str_ln("save_flags");
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
        (tempRelays.top != currentRelays[currentAntenna].top) ||
        (tempRelays.bot != currentRelays[currentAntenna].bot) ||
        (temp_flags.flags != system_flags.flags))
    {
        #if LOG_LEVEL_FLAGS >= LOG_INFO
        print_str_ln("  saving records");
        #endif
        
        internal_eeprom_write(i, 0xff);
        
        i += 4;
        if (i > 248) i = 0;

        internal_eeprom_write(i, (swrThreshIndex & 0x7f));
        internal_eeprom_write(i + 1, currentRelays[currentAntenna].top);
        internal_eeprom_write(i + 2, currentRelays[currentAntenna].bot);
        internal_eeprom_write(i + 3, system_flags.flags);
    }
}
