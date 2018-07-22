#ifndef _NONVOLATILE_MEMORY_H_
#define _NONVOLATILE_MEMORY_H_

#include <stdint.h>

/* ************************************************************************** */

extern void nonvolatile_memory_init(void);

/* ************************************************************************** */

/*  Notes on using EEPROM Memory operations:
    
    EEPROM can be read AND written as individual bytes, no block operations are
    needed.
    
    eeprom_read() is very fast, the read operation itself takes a single cycle.
    
    eeprom_write() can be quite a bit slower.  The write operation can take as 
    long as 4ms to complete.  eeprom_write() does NOT wait for the write
    operation to complete, instead returning as soon as the write is engaged.
    In order to prevent conflict with multiple back-to-back writes, the relevant
    flag is checked at the beginning of the function.  If a previous write is
    still in process, it will wait until it completes before continuing with the
    new write operation.
    
    In order to preserve the limited number of EEPROM write/erase cycles,
    eeprom_write() first reads the provided address and compares the stored
    value with the value that is to be stored.  It only writes if the two do not
    match.
    
    PIC18F2X/4XK22 devices have either 256 or 1024 bytes of EEPROM.  The address
    is an unsigned 16 bit integer to accomodate the larger devices.  If a model
    with 256 bytes of EEPROM is being used, feel free to change this code to
    use a uint8_t.
*/

// Read one byte from EEPROM memory at (address)
extern uint8_t internal_eeprom_read(uint16_t address);

// Write one byte to EEPROM memory at (address)
extern void internal_eeprom_write(uint16_t address, uint8_t c);

/* ************************************************************************** */

/*  Notes on using Flash Memory operations on the PIC18F4520
    
    Flash memory can be read as individual bytes, must be erased in 64 byte
    blocks, and can only be written in 32 byte blocks.
    
    The flash_block_xxxx() functions are designed to facilitate this block
    behavior.  They automatically align all operations to block boundaries, so
    any address in a given block will produce the same results.
    
    Recommended procedure for writing to flash memory is as follows:
    
    void example_write(NVM_address_t address, uint8_t value1, uint8_t value2)
    {
        uint8_t buffer[64];
        uint8_t *pointer;
        uint8_t i;
    
        pointer = buffer;
        
        // Read existing block into buffer
        flash_block_read(address, pointer);
        
        // Mask off everything but bottom 6 bits(64 addresses)
        i = address & 0x003f;
        
        // Write the two new values into the buffer at the correct addresses
        buffer[i] = value1;
        buffer[i + 1] = value2;    
        
        // Write the edited buffer into flash    
        flash_block_write(address, pointer);
    }
*/

typedef __uint24 NVM_address_t;

// Read one byte from Flash memory at (address)
uint8_t flash_read(NVM_address_t address);

// Read an entire block of 64 bytes from Flash memory into the provided buffer
void flash_block_read(NVM_address_t address, uint8_t *buffer);

// Erase a block of Flash memory at (address)
void flash_block_erase(NVM_address_t address);

// Write the provided buffer into flash memory at (address)
void flash_block_write(NVM_address_t address, uint8_t *buffer);


#endif