#include "../includes.h"

/* ************************************************************************** */

/*  nvm_write() is the 'engage' button for writing to both types of nonvolatile
    memory.  
    
    This function is the smallest unit that needs to be marked critical.
*/
void nvm_write(void)
{
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

uint8_t internal_eeprom_read(uint16_t address)
{
    #if LOG_LEVEL_EEPROM >= LOG_INFO
    println("eeprom_read");
    #endif
    
    // Load lower byte of address into register
    NVMADRL = address;
    // If EEADRH is present, load high byte of address into register
    #if EEADRH
    NVMADRH = address >> 8;
    #endif
    
    // Select EEPROM
    NVMCON1bits.REG = 0;
    // Initiate read operation
    NVMCON1bits.RD = 1;
    
    // Return the value
    return (NVMDAT);
}

void internal_eeprom_write(uint16_t address, uint8_t value)
{
    #if LOG_LEVEL_EEPROM >= LOG_INFO
    println("eeprom_write");
    #endif
    // Wait for possible previous write to complete
    while(NVMCON1bits.WR);
    
    // Load lower byte of address into register
    NVMADRL = address;
    // If EEADRH is present, load high byte of address into register
    #if EEADRH
    NVMADRH = address >> 8;
    #endif
    
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

uint8_t flash_read(uint32_t address)
{
    #if LOG_LEVEL_FLASH >= LOG_INFO
    println("flash_read");
    #endif
    
    // Load the address into the tablepointer registers
    TBLPTRL = address;
    TBLPTRH = address >> 8;
    TBLPTRU = address >> 16;

    // Read one byte at the given address
    asm("TBLRD*+");

    return TABLAT;
}

void flash_block_read(uint32_t address, uint8_t *buffer)
{
    uint8_t i = 64;
    uint32_t blockAddress;
    
    #if LOG_LEVEL_FLASH >= LOG_INFO
    println("flash_block_read");
    #endif
    
    // Mask off the block address
    blockAddress = address & 0xffffffc0;
    
    // Load the address into the tablepointer registers
    TBLPTRL = blockAddress;
    TBLPTRH = blockAddress >> 8;
    TBLPTRU = blockAddress >> 16;

    while(i--)
    {
        asm("TBLRD*+");
        *buffer++ = TABLAT;
    }
}

void flash_block_erase(uint32_t address)
{
    #if LOG_LEVEL_FLASH >= LOG_INFO
    println("flash_block_erase");
    #endif
    
    // Load the address into the tablepointer registers
    TBLPTRL = address;
    TBLPTRH = address >> 8;
    TBLPTRU = address >> 16;

    // Helmsman, engage
    NVMCON1bits.REG = 1;
    NVMCON1bits.FREE = 1;
    nvm_write();
}

void flash_block_write(uint32_t address, uint8_t *buffer)
{
    uint8_t i = 0;
    uint32_t blockAddress;
    
    #if LOG_LEVEL_FLASH >= LOG_INFO
    println("flash_block_write");
    #endif

    // Mask off the block address
    blockAddress = address & 0xffffffc0;
    
    #if LOG_LEVEL_FLASH >= LOG_DETAILS
    printf("address: %d, blockAddress: %d", address, blockAddress);
    #endif
    
    // Load the address into the tablepointer registers
    TBLPTRL = blockAddress;
    TBLPTRH = blockAddress >> 8;
    TBLPTRU = blockAddress >> 16;

    // Load the first half-block into the write buffer
    while(i < 32)
    {
        TABLAT = buffer[i++];
        asm("TBLWT*+");
    }
    
    // Decrement the tablepointer before nvm_write()
    asm("TBLRD*-");
    
    // Helmsman, engage
    NVMCON1bits.REG = 0b10;
    NVMCON1bits.FREE = 0;
    nvm_write();

    // Incrememnt the tablepointer to put it back where it was
    asm("TBLRD*+");
    
    // Load the second half-block into the write buffer
    while(i < 64)
    {
        TABLAT = buffer[i++];
        asm("TBLWT*+");
    }
    
    // Decrement the tablepointer before nvm_write()
    asm("TBLRD*-");
    
    // Helmsman, engage
    NVMCON1bits.REG = 0b10;
    NVMCON1bits.FREE = 0;
    nvm_write();
}