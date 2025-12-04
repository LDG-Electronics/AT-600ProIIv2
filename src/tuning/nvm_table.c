#include "nvm_table.h"
#include "flags.h"
#include "os/logging.h"
#include "peripherals/nonvolatile_memory.h"
#include <stdbool.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

// returns a correctly initialized table_entry_t object
table_entry_t new_table_entry(void) {
    table_entry_t entry;

    memset(&entry, 0, sizeof(table_entry_t));

    return entry;
}

void print_table_entry(table_entry_t entry) {
    for (uint8_t i = 0; i < sizeof(table_entry_t); i++) {
        printf("[%02x]", entry.contents[i]);
    }
}

// define a giant array in ROM to reserve space for saving tune memories
const table_entry_t nvmTable[NUMBER_OF_TABLE_ENTRIES] __at(TABLE_LOCATION) = {};

/* ************************************************************************** */

static void print_configuration_data(void) {
    println("");
    printf("TABLE_ENTRY_SIZE: %d\r\n", TABLE_ENTRY_SIZE);
    printf("NUMBER_OF_TABLE_ENTRIES: %d\r\n", NUMBER_OF_TABLE_ENTRIES);
    printf("TABLE_SIZE: %d\r\n", TABLE_SIZE);
    printf("TABLE_LOCATION: %ld\r\n", TABLE_LOCATION);

    printf("tableLocation: %lu\r\n", (uint32_t)&nvmTable[0]);
}

void nvm_table_init(void) {
    //
    nonvolatile_memory_init();

    // register with the logging subsystem
    log_register();

    //
    LOG_DEBUG({ print_configuration_data(); });
}

/* ************************************************************************** */

static NVM_address_t resolve_table_address(uint16_t slot) {
    LOG_TRACE({ println("resolve_table_address"); });

    NVM_address_t address = (NVM_address_t)&nvmTable[slot];

    LOG_DEBUG({
        printf("slot: %u ", slot);
        print_nvm_address_ln(address);
        println("");
    });

    return address;
}

/* ************************************************************************** */

// Recall a table_entry_t object from the specified slot
table_entry_t nvm_table_read(uint16_t slot) {
    LOG_TRACE({ println("nvm_table_read"); });

    NVM_address_t address = resolve_table_address(slot);
    table_entry_t entry = new_table_entry();

    // copy the contents of flash at <address> into the table_entry_t object
    for (uint8_t i = 0; i < TABLE_ENTRY_SIZE; i++) {
        entry.contents[i] = flash_read_byte(address + i);
    }

    LOG_INFO({
        print_nvm_address(address);
        print("entry: ");
        print_table_entry(entry);
        println("");
    });

    return entry;
}

/* -------------------------------------------------------------------------- */

// compare the two variables bit-by-bit, checking for 0->1 transitions
static bool must_erase(table_entry_t new, table_entry_t old) {
    // we have to compare every element of each entry
    for (uint8_t i = 0; i < TABLE_ENTRY_SIZE; i++) {
        // and every bit of each element
        for (uint8_t k = 0; k < 8; k++) {
            if (!(old.contents[i] & (1 << k)) && (new.contents[i] & (1 << k))) {
                return true;
            }
        }
    }
    return false;
}

// Store the provided entry object in the specified slot
void nvm_table_write(uint16_t slot, table_entry_t newEntry) {
    LOG_TRACE({ println("nvm_table_write"); });

    NVM_address_t address = resolve_table_address(slot);

    // Pull out what's already at the specified address
    table_entry_t existingEntry = nvm_table_read(slot);

    LOG_INFO({
        print_nvm_address(address);
        print("newEntry: ");
        print_table_entry(newEntry);
        print(" existingEntry: ");
        print_table_entry(existingEntry);
        println("");
    });

    bool entriesAreDifferent = false;

    // Make sure we aren't wasting an erase/write cycle
    for (uint8_t i = 0; i < TABLE_ENTRY_SIZE; i++) {
        if (newEntry.contents[i] != existingEntry.contents[i]) {
            entriesAreDifferent = true;
            break;
        }
    }

    if (!entriesAreDifferent) {
        LOG_INFO({ println("newEntry matches existingEntry"); });
        return;
    }

    // Fill the buffer with the current flash contents
    LOG_INFO({ println("Reading existing block"); });
    uint8_t buffer[FLASH_BUFFER_SIZE];
    flash_read_block(address, buffer);

    // identify which element in the array corresponds to our address
    uint8_t element = address & FLASH_ELEMENT_MASK;

    // update the buffer with our new data
    for (uint8_t i = 0; i < TABLE_ENTRY_SIZE; i++) {
        buffer[element + i] = newEntry.contents[i];
    }

    // erase, but only if necessary
    if (must_erase(newEntry, existingEntry)) {
        LOG_INFO({ println("Erasing block"); });
        flash_erase_block(address);
    }

    // finally, we can write our modified block back into flash
    LOG_INFO({ println("Writing new block"); });
    flash_write_block(address, buffer);
}