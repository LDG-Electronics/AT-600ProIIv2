#include "nonvolatile_memory.h"
#include "../os/log_macros.h"
#include "interrupt.h"
#include "pic18f46k42.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void nonvolatile_memory_init(void) {
    log_register();
    LOG_TRACE({ printf("_FLASH_ERASE_SIZE: %d\r\n", _FLASH_ERASE_SIZE); });
    LOG_TRACE({ printf("_FLASH_WRITE_SIZE: %d\r\n", _FLASH_WRITE_SIZE); });
}

/* ************************************************************************** */

/*  nvm_write() is the 'engage' button for writing to both types of nonvolatile
    memory.

    This function is the smallest unit that needs to be marked critical.
*/
void nvm_write(void) {
    begin_critical_section();
    NVMCON1bits.WREN = 1; // Enable NVM writes

    // Magic Sequence - Do Not Change
    NVMCON2 = 0x55;
    NVMCON2 = 0xAA;
    NVMCON1bits.WR = 1;

    NVMCON1bits.WREN = 0; // Disable NVM writes
    end_critical_section();
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
    return NVMDAT;
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

uint8_t FLASH_ReadByte(uint32_t flashAddr) {
    NVMCON1bits.NVMREG = 2;
    TBLPTRU = (uint8_t)((flashAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t)((flashAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t)(flashAddr & 0x000000FF);

    asm("TBLRD");

    return (TABLAT);
}

uint16_t FLASH_ReadWord(uint32_t flashAddr) {
    return ((((uint16_t)FLASH_ReadByte(flashAddr + 1)) << 8) |
            (FLASH_ReadByte(flashAddr)));
}

uint8_t flash_read(NVM_address_t address) {
    LOG_TRACE({ println("flash_read"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = address;

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Read one byte at the given address
    asm("TBLRD*+");

    return TABLAT;
}

#define BLOCK_MASK(ADDRESS) address & 0xffffc0

void flash_block_read(NVM_address_t address, uint8_t *readBuffer) {
    LOG_TRACE({ println("flash_block_read"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = BLOCK_MASK(address);

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Read out the block into the readBuffer
    for (uint8_t i = 0; i < _FLASH_WRITE_SIZE; i++) {
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

void FLASH_EraseBlock(uint32_t baseAddr) {
    uint8_t GIEBitValue = INTCON0bits.GIE; // Save interrupt enable

    TBLPTRU = (uint8_t)((baseAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t)((baseAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t)(baseAddr & 0x000000FF);

    NVMCON1bits.NVMREG = 2;
    NVMCON1bits.WREN = 1;
    NVMCON1bits.FREE = 1;
    asm("BCF INTCON0,7");
    asm("BANKSEL NVMCON1");
    asm("BSF NVMCON1,2");
    asm("MOVLW 0x55");
    asm("MOVWF NVMCON2");
    asm("MOVLW 0xAA");
    asm("MOVWF NVMCON2");
    asm("BSF NVMCON1,1");
    asm("BSF INTCON0,7");
}

void flash_block_write(NVM_address_t address, uint8_t *writeBuffer) {
    LOG_TRACE({ println("flash_block_write"); });
    LOG_DEBUG({ printf("address: %ul\r\n", address); });

    // Load the address into the tablepointer registers
    TBLPTR = BLOCK_MASK(address);

    uint32_t temp = TBLPTR;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", temp); });

    // Load the block into the writeBuffer
    for (uint8_t i = 0; i < _FLASH_WRITE_SIZE; i++) {
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

void FLASH_WriteByte(uint32_t flashAddr, uint8_t *flashRdBufPtr, uint8_t byte) {
    uint32_t blockStartAddr =
        (uint32_t)(flashAddr & ((END_FLASH - 1) ^ (ERASE_FLASH_BLOCKSIZE - 1)));
    uint8_t offset = (uint8_t)(flashAddr & (ERASE_FLASH_BLOCKSIZE - 1));
    uint8_t i;

    // Entire row will be erased, read and save the existing data
    for (i = 0; i < ERASE_FLASH_BLOCKSIZE; i++) {
        flashRdBufPtr[i] = FLASH_ReadByte((blockStartAddr + i));
    }

    // Load byte at offset
    flashRdBufPtr[offset] = byte;

    // Writes buffer contents to current block
    FLASH_WriteBlock(blockStartAddr, flashRdBufPtr);
}

int8_t FLASH_WriteBlock(uint32_t writeAddr, uint8_t *flashWrBufPtr) {
    uint32_t blockStartAddr =
        (uint32_t)(writeAddr & ((END_FLASH - 1) ^ (ERASE_FLASH_BLOCKSIZE - 1)));
    uint8_t GIEBitValue = INTCON0bits.GIE; // Save interrupt enable
    uint8_t i;

    // Flash write must start at the beginning of a row
    if (writeAddr != blockStartAddr) {
        return -1;
    }

    // Block erase sequence
    FLASH_EraseBlock(writeAddr);

    // Block write sequence
    TBLPTRU = (uint8_t)((writeAddr & 0x00FF0000) >> 16);
    TBLPTRH = (uint8_t)((writeAddr & 0x0000FF00) >> 8);
    TBLPTRL = (uint8_t)(writeAddr & 0x000000FF);

    // Write block of data
    for (i = 0; i < WRITE_FLASH_BLOCKSIZE; i++) {
        TABLAT = flashWrBufPtr[i]; // Load data byte

        if (i == (WRITE_FLASH_BLOCKSIZE - 1)) {
            asm("TBLWT");
        } else {
            asm("TBLWTPOSTINC");
        }
    }

    NVMCON1bits.NVMREG = 2;
    NVMCON1bits.WREN = 1;
    asm("BCF INTCON0,7");
    asm("BANKSEL NVMCON1");
    asm("BSF NVMCON1,2");
    asm("MOVLW 0x55");
    asm("MOVWF NVMCON2");
    asm("MOVLW 0xAA");
    asm("MOVWF NVMCON2");
    asm("BSF NVMCON1,1");
    asm("BSF INTCON0,7");
    asm("BCF NVMCON1,2");

    return 0;
}
