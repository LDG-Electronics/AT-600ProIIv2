#include "flags.h"
#include "os/log_macros.h"
#include "peripherals/nonvolatile_memory.h"
#include "relay_driver.h"
#include "relays.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */
//! READ THIS BEFORE TRYING TO UNDERSTAND THIS FILE
/*  Flag Manager theory of operation:

    The system has a handful of centrally managed systemFlags to keep track of
    the state of the unit. Yes, systemFlags is a global variable.

    The contents of these flags needs to persist across power loss. The best way
    to do this is storing flags in EEPROM. While we're at it, there are several
    other system settings that should also be stored in EEPROM.

    We need to store:
    SWR Threshold Index, from RF_sensor.h
    systemFlags, from flags.h
    currentRelays and preBypassRelays, from relays.h

    The collection of all of these fields is called a 'record'. There's a record
    data type, flag_record_t, that will assist us in saving/restoring these
    variables.

    EEPROM has a limited lifespan. The PIC18FXXK42 datasheet lists EEPROM write
    endurance as 100k erase/write cycles. In order to lengthen product lifespan,
    we're using a simple rotating wear-leveling technique. There are 20 'slots'
    allocated in EEPROM, and instead of updating a single record at a single
    EEPROM location, we iterate through the record slots. This spreads out the
    erase cycles across a larger number of EEPROM cells, therefore making each
    cell last longer.

    The current record is marked by the most-significant-bit of the first byte.
    A 0 in that spot means that it's the most recent, a 1 in that spot means
    it's... not the most recent.
*/
//! SERIOUSLY, READ THIS FIRST

system_flags_t systemFlags;

/* -------------------------------------------------------------------------- */
/*  system_flag_bits_t is a data used to pack the full sized system flags into
    a form that's efficient to store in EEPROM

*/
typedef struct {
    unsigned ant1Bypass : 1;
    unsigned ant2Bypass : 1;
    unsigned antenna : 1;
    unsigned autoMode : 1;
    unsigned peakMode : 1;
    unsigned scaleMode : 1;
    unsigned powerStatus : 1;
} system_flag_bits_t;

system_flag_bits_t pack_system_flags(void) {
    system_flag_bits_t flagBits;

    flagBits.ant1Bypass = systemFlags.bypassStatus[0];
    flagBits.ant2Bypass = systemFlags.bypassStatus[1];
    flagBits.antenna = systemFlags.antenna;
    flagBits.autoMode = systemFlags.autoMode;
    flagBits.peakMode = systemFlags.peakMode;
    flagBits.scaleMode = systemFlags.scaleMode;
    flagBits.powerStatus = systemFlags.powerStatus;

    return flagBits;
}

void unpack_system_flags(system_flag_bits_t flagBits) {
    systemFlags.bypassStatus[0] = flagBits.ant1Bypass;
    systemFlags.bypassStatus[1] = flagBits.ant2Bypass;
    systemFlags.antenna = flagBits.antenna;
    systemFlags.autoMode = flagBits.autoMode;
    systemFlags.peakMode = flagBits.peakMode;
    systemFlags.scaleMode = flagBits.scaleMode;
    systemFlags.powerStatus = flagBits.powerStatus;
}

/* ************************************************************************** */
/*  flag_record_fields_t

    This is kind of a hot mess, but there's a good reason. Manually counting
    struct sizes is generally a bad idea. Struct size is not always obvious, and
    a defined literal is always at risk of falling out-of-sync with the struct
    definition.

    Instead, FLAG_RECORD_SIZE is determined with the sizeof() operator. Using
    sizeof() causes a bit of a chicken-and-egg problem, because the value needs
    to be used INSIDE the flag_record_t struct, whose definition can't be known
    until FLAG_RECORD_SIZE is defined.

    The workaround is to define the actual fields in flag_record_fields_t, and
    then add that to flag_record_t as an anonymous member. This is apparently
    gross and non-portable, but it lets me use sizeof().
*/
typedef struct {
    uint8_t threshIndex;
    system_flag_bits_t flagBits;
    packed_relays_t relayBits[NUM_OF_ANTENNA_PORTS * 2];
} flag_record_fields_t;

/*  flag_record_t is syntactic sugar used to streamline the saving and restoring
    of general system status to/from EEPROM.

    The array[] member is a trick to gain bytewise access to the entire struct,
    so we can iterate through it as we read or write one byte of EEPROM at a
    time.
*/
#define FLAG_RECORD_SIZE (uint8_t)sizeof(flag_record_fields_t)
typedef union {
    flag_record_fields_t fields;
    uint8_t array[FLAG_RECORD_SIZE];
} flag_record_t;

/*  K42 series PICs will either have 256 or 1024 bytes of EEPROM, but we're
    probably using one with 1024.

    Therefore if(!) FLAG_RECORD_SIZE is 10, 20 record slots will consume 200
    bytes of EEPROM.
*/
#define NUMBER_OF_RECORD_SLOTS 20

/* ************************************************************************** */

void flags_init(void) {
    nonvolatile_memory_init();

    // populate systemFlags with default values
    systemFlags.bypassStatus[0] = 1; // default is bypass
    systemFlags.bypassStatus[1] = 1; // default is bypass
    systemFlags.antenna = 0;         // default is Ant 2
    systemFlags.autoMode = 0;        // default is semi mode
    systemFlags.peakMode = 0;        // default is NOT peak mode
    systemFlags.scaleMode = 1;       // default is 1, full scale
    systemFlags.powerStatus = 1;     // default is 1

    swrThreshIndex = 0;

    log_register();
}

/* ************************************************************************** */

#define NO_VALID_RECORD -1

/*  find_current_record() steps through EEPROM, reading the first byte of
    every stored record, looking for one where the first bit is 0.
*/
int16_t find_current_record(void) {
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

/* ************************************************************************** */

// reads EEPROM at the given address and returns a populated record
flag_record_t read_record(uint16_t address) {
    flag_record_t record;

    // mask off the record
    record.array[0] = (internal_eeprom_read(address) & 0x7f);

    // remaining elements can be read directly
    for (uint8_t i = 1; i < FLAG_RECORD_SIZE; i++) {
        record.array[i] = (internal_eeprom_read(address + i));
    }

    return record;
}

// copy record fields back to their homes in the system
void unpack_record_to_system(flag_record_t *record) {
    // these fields can be read directly
    swrThreshIndex = record->fields.threshIndex;
    unpack_system_flags(record->fields.flagBits);

    // these fields need to be unpacked
    currentRelays[0] = unpack_relays(record->fields.relayBits[0]);
    currentRelays[1] = unpack_relays(record->fields.relayBits[1]);
    preBypassRelays[0] = unpack_relays(record->fields.relayBits[2]);
    preBypassRelays[1] = unpack_relays(record->fields.relayBits[3]);
}

// restore most recently saved system settings from EEPROM
void load_flags(void) {
    LOG_TRACE({ println("load_flags"); });

    int16_t address = find_current_record();

    if (address != NO_VALID_RECORD) {
        LOG_INFO({ printf("found valid records at: %d\r\n", address); });

        flag_record_t existingRecord = read_record(address);
        unpack_record_to_system(&existingRecord);

    } else {
        LOG_INFO({ println("no valid record"); });
    }
}

/* -------------------------------------------------------------------------- */

// writes the provided record to EEPROM at the given address
void write_record(flag_record_t *record, uint16_t address) {
    // first element needs to be modified
    internal_eeprom_write(address, (record->array[0] & 0x7f));

    // remaining elements can be written directly
    for (uint8_t i = 1; i < FLAG_RECORD_SIZE; i++) {
        internal_eeprom_write(address + i, record->array[i]);
    }
}

// assemble a record from various system status variables
flag_record_t pack_system_to_record(void) {
    flag_record_t record;

    record.fields.threshIndex = swrThreshIndex;
    record.fields.flagBits = pack_system_flags();

    // these fields need to be packed
    record.fields.relayBits[0] = pack_relays(currentRelays[0]);
    record.fields.relayBits[1] = pack_relays(currentRelays[1]);
    record.fields.relayBits[2] = pack_relays(preBypassRelays[0]);
    record.fields.relayBits[3] = pack_relays(preBypassRelays[1]);

    return record;
}

// save current system settings to EEPROM
// TODO: if this is called too often, it fails to detect matching records
// Ian suggests looking at discrepencies in antenna selection
void save_flags(void) {
    LOG_TRACE({ println("save_flags"); });

    int16_t address = find_current_record();
    // if no record found, then we're saving to address 0
    if (address == NO_VALID_RECORD) {
        address = 0;
    }

    // assemble our records
    flag_record_t existingRecord = read_record(address);
    flag_record_t newRecord = pack_system_to_record();

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