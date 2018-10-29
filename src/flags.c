#include "flags.h"
#include "os/log_macros.h"
#include "peripherals/nonvolatile_memory.h"
#include "peripherals/relay_driver.h"
#include "relays.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

#define NUMBER_OF_RECORD_SLOTS 15
#define FLAG_RECORD_SIZE 10
typedef union {
    struct {
        uint8_t threshIndex;
        system_flags_t flags;
        packed_relays_t relayBits[NUM_OF_ANTENNA_PORTS * 2];
    };
    uint8_t array[FLAG_RECORD_SIZE];
} flag_record_t;

system_flags_t systemFlags;

uint8_t bypassStatus[NUM_OF_ANTENNA_PORTS];

/* ************************************************************************** */

void flags_init(void) {
    nonvolatile_memory_init();

    // populate systemFlags with default values
    systemFlags.ant1Bypass = 1;  // default value is bypass
    systemFlags.ant2Bypass = 1;  // default value is bypass
    systemFlags.antenna = 0;     // default value is Ant 2
    systemFlags.autoMode = 0;    // default value is semi mode
    systemFlags.peakMode = 0;    // default value is NOT peak mode
    systemFlags.scaleMode = 0;   // default value is
    systemFlags.powerStatus = 1; // default value is 1
    swrThreshIndex = 0;

    // copy stored bypass values to the usable array
    bypassStatus[0] = systemFlags.ant1Bypass;
    bypassStatus[1] = systemFlags.ant2Bypass;

    log_register();
}

/* ************************************************************************** */

// reads EEPROM at the given address and returns a populated record
flag_record_t read_record(uint8_t address) {
    flag_record_t record;

    // first element needs to be modified
    record.threshIndex = (internal_eeprom_read(address) & 0x7f);

    // remaining elements can be read directly
    for (uint8_t i = 1; i < FLAG_RECORD_SIZE; i++) {
        record.array[i] = (internal_eeprom_read(address + i));
    }

    return record;
}

// writes the provided record to EEPROM at the given address
void write_record(flag_record_t *record, uint8_t address) {
    // first element needs to be modified
    internal_eeprom_write(address, (record->threshIndex & 0x7f));

    // remaining elements can be written directly
    for (uint8_t i = 1; i < FLAG_RECORD_SIZE; i++) {
        internal_eeprom_write(address + i, record->array[i]);
    }
}

/* -------------------------------------------------------------------------- */

#define NO_VALID_RECORD -1

/*  identify_valid_record() steps through EEPROM, reading the first byte of
    every stored record, looking for one where the first bit is 0.

    When a new record is written, the previous record's first bit is set to 1.
    In this way, only one record should have its first bit cleared, thereby
    denoting the most recently saved record.
*/
int16_t identify_valid_record(void) {
    uint8_t address = 0;

    while (address < (FLAG_RECORD_SIZE * NUMBER_OF_RECORD_SLOTS)) {
        if ((internal_eeprom_read(address) & 0x80) == 0) {
            return address;
        }
        address += FLAG_RECORD_SIZE;
    }

    // didn't find record
    return NO_VALID_RECORD;
}

/* -------------------------------------------------------------------------- */

// restore most recently saved system settings from EEPROM
void load_flags(void) {
    LOG_TRACE({ println("load_flags"); });

    int16_t address = identify_valid_record();

    if (address != NO_VALID_RECORD) {
        LOG_INFO({ printf("found valid records at: %d\r\n", address); });

        flag_record_t existingRecord = read_record(address);

        // load record back into the world
        swrThreshIndex = existingRecord.threshIndex;
        systemFlags = existingRecord.flags;
        currentRelays[0] = unpack_relays(&existingRecord.relayBits[0]);
        currentRelays[1] = unpack_relays(&existingRecord.relayBits[1]);
        preBypassRelays[0] = unpack_relays(&existingRecord.relayBits[2]);
        preBypassRelays[1] = unpack_relays(&existingRecord.relayBits[3]);

        // copy stored bypass values to the usable array
        // Ant relay is wired backwards: 0 is ANT2, 1 is ANT1
        bypassStatus[0] = systemFlags.ant2Bypass;
        bypassStatus[1] = systemFlags.ant1Bypass;
    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

/* -------------------------------------------------------------------------- */

// save current system settings to EEPROM
// TODO: if this is called too often, it fails to detect matching records
void save_flags(void) {
    LOG_TRACE({ println("save_flags"); });

    // copy usable bypass array values back into flags
    // Ant relay is wired backwards: 0 is ANT2, 1 is ANT1
    systemFlags.ant2Bypass = bypassStatus[0];
    systemFlags.ant1Bypass = bypassStatus[1];

    // grab a copy of the 'world', such as it is
    flag_record_t newRecord;
    newRecord.threshIndex = swrThreshIndex;
    newRecord.flags = systemFlags;
    newRecord.relayBits[0] = pack_relays(&currentRelays[0]);
    newRecord.relayBits[1] = pack_relays(&currentRelays[1]);
    newRecord.relayBits[2] = pack_relays(&preBypassRelays[0]);
    newRecord.relayBits[3] = pack_relays(&preBypassRelays[1]);

    int16_t address = identify_valid_record();
    // if no record found, then we're saving to address 0
    if (address == NO_VALID_RECORD) {
        address = 0;
    }

    // grab the most recent record
    flag_record_t existingRecord = read_record(address);

    // make sure what we're trying to save is different
    bool recordsAreDifferent = false;
    for (uint8_t i = 0; i < FLAG_RECORD_SIZE; i++) {
        if (existingRecord.array[i] != newRecord.array[i]) {
            recordsAreDifferent = true;
        }
    }

    if (recordsAreDifferent) {
        // invalidate the old record
        internal_eeprom_write(address, 0xff);

        // increment to the next record location
        address += FLAG_RECORD_SIZE;
        if (address > (FLAG_RECORD_SIZE * NUMBER_OF_RECORD_SLOTS)) {
            address = 0;
        }

        LOG_INFO({ printf("saving records at: %d\r\n", address); });
        write_record(&newRecord, address);
    } else {
        LOG_INFO({ println("new record matched existing record"); });
    }
}