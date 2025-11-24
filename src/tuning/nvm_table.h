#ifndef _NVM_TABLE_H_
#define _NVM_TABLE_H_

#include <stdint.h>

/* ************************************************************************** */

// The size, in bytes, of a single table entry
#define TABLE_ENTRY_SIZE 8

/*  table_entry_t is an intermediate data type used to prevent this module from
    having unnecessary dependencies on project specific code.

*/
typedef struct {
    uint8_t contents[TABLE_ENTRY_SIZE];
} table_entry_t;

// returns a correctly initialized table_entry_t object
extern table_entry_t new_table_entry(void);

// print a table_entry_t object
extern void print_table_entry(table_entry_t entry);

/* ************************************************************************** */

// this was determined by running print_all_frequency_groups()
#define NUMBER_OF_TABLE_ENTRIES 3500

// I hope this is obvious
#define TABLE_SIZE (NUMBER_OF_TABLE_ENTRIES * TABLE_ENTRY_SIZE)

// Align the table towards the end of ROM
// Used the compiler-defined _ROMSIZE so we're device independant
// Use FLASH_BLOCK_MASK to make sure the array is block-aligned
// offset the table by FLASH_ERASE_BLOCKSIZE to make sure it's in bounds
#define TABLE_LOCATION ((_ROMSIZE - (TABLE_SIZE)) & FLASH_BLOCK_MASK) - FLASH_ERASE_BLOCKSIZE

/* ************************************************************************** */

// setup
extern void nvm_table_init(void);

// Recall a table_entry_t object from the specified slot
extern table_entry_t nvm_table_read(uint16_t slot);

// Store the provided table_entry_t object in the specified slot
extern void nvm_table_write(uint16_t slot, table_entry_t newEntry);

#endif // _NVM_TABLE_H_