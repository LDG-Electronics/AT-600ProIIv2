#include "nonvolatile_memory.h"
#include "../os/log_macros.h"
#include "pic18f46k42.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void nonvolatile_memory_init(void) { log_register(); }

/* ************************************************************************** */

/*  nvm_write() is the 'engage' button for writing to both types of nonvolatile
    memory.

    This function is the smallest unit that needs to be marked critical.
*/
void nvm_write(void) {
    NVMCON1bits.WREN = 1; // Enable NVM writes

    // Magic Sequence - Do Not Change
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;

    NVMCON1bits.WREN = 0; // Disable NVM writes
}

/*  Notes on PIC18's EEPROM Memory:

    EEPROM in PIC18 family MCUs is very straightforward.
    There is 256 or 1024 bytes of byte-addressable EEPROM.

    Both read and write operations follow the same basic sequence, with the
    major difference being time required.  Reading from EEPROM only takes one
    cycle after the operation is started.  Writing to EEPROM takes up to 4ms.
    Other user code could be executed during that time, but in this application
    we will choose to wait for it to finish.

    Read:
    1) Load the desired address into EEADR(and EEADRH)
    2) Set the RD bit of EECON1, to begin operation
    3) Read byte from EEDATA

    Write:
    1) Load the desired address into EEADR(and EEADRH)
    2) Load the byte to be written into EEDATA
    3) Set the WREN bit of EECON1, to enable writes
    4) Write the 'magic sequence' of 0x55, 0xAA to EECON2, to unlock writes
    5) Set the WR bit of EECON1, to begin operation
    6) Wait for WR bit to be cleared by hardware
*/

uint8_t internal_eeprom_read(uint16_t address) {
    LOG_TRACE({ println("eeprom_read"); });

    NVMADRL = address;
    NVMADRH = address >> 8;

    // Select EEPROM
    NVMCON1bits.REG = 0;
    // Initiate read operation
    NVMCON1bits.RD = 1;

    // Return the value
    return (NVMDAT);
}

void internal_eeprom_write(uint16_t address, uint8_t value) {
    LOG_TRACE({ println("eeprom_write"); });
    // Wait for possible previous write to complete
    if (NVMCON1bits.WR) {
        LOG_DEBUG({ println("previous write not finished"); });
    }
    while (NVMCON1bits.WR) {
        // empty
    }

    NVMADRL = address;
    NVMADRH = address >> 8;

    // Load value into register
    NVMDAT = value;

    // Select EEPROM
    NVMCON1bits.REG = 0;

    // Engage
    nvm_write();
}

/* ************************************************************************** */

/*  Notes on PIC18's Flash memory:

    Flash memory can be read as individual bytes by loading a desired address
    into TBLPTR and executing a TBLRD* asm instruction.  The result of the read
    operation is placed into the TABLAT register.

    Flash memory can only be written in 64 byte blocks.  In order to write only
    one or two bytes to flash without corrupting surrounding values, the
    following steps are necessary:
        1) Declare a buffer to hold 64 bytes.
        2) Call flash_block_read() with the desired address and buffer.
        3) Modify the buffer's contents with the new values.
        4) Call flash_block_write() to write the new buffer into the block.
        5) Verify that the block was written to correctly.

    Registers involved in Flash memory operations:

    TBLPTR:

    TABLAT:

    EECON1: control bits for Flash operations
        EEPGD: Select Flash or EEPROM
        CFGS: Select Flash/EEPROM or Config Bits
        n/a
        FREE:
        WRERR:
        WREN:
        WR: Write Control, set to initiate write
        RD: Read Control, set to initiate read
    EECON2: recieves the 'magic sequence' to allow flash writes

*/

uint8_t flash_read(NVM_address_t address) {
    LOG_TRACE({ println("flash_read"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = address;

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", TBLPTR); });

    // Read one byte at the given address
    asm("TBLRD*+");

    return TABLAT;
}

#define FLASH_BLOCK_SIZE 64
#define BLOCK_MASK(ADDRESS) address & 0xffffc0

void flash_block_read(NVM_address_t address, uint8_t *readBuffer) {
    LOG_TRACE({ println("flash_block_read"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = BLOCK_MASK(address);

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Read out the block into the readBuffer
    for (uint8_t i = 0; i < FLASH_BLOCK_SIZE; i++) {
        asm("TBLRD*+");
        readBuffer[i] = TABLAT;
    }
}

void flash_block_erase(NVM_address_t address) {
    LOG_TRACE({ println("flash_block_erase"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = address;

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Helmsman, engage
    NVMCON1bits.REG = 1;
    NVMCON1bits.FREE = 1;
    nvm_write();
}

void flash_block_write(NVM_address_t address, uint8_t *writeBuffer) {
    LOG_TRACE({ println("flash_block_write"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = BLOCK_MASK(address);

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Load the block into the writeBuffer
    for (uint8_t i = 0; i < FLASH_BLOCK_SIZE; i++) {
        TABLAT = writeBuffer[i++];
        asm("TBLWT*+");
    }

    // Decrement the tablepointer before nvm_write()
    asm("TBLRD*-");

    // Helmsman, engage
    NVMCON1bits.REG = 0b10;
    NVMCON1bits.FREE = 0;
    nvm_write();
}