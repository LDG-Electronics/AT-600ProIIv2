#include "nonvolatile_memory.h"
#include "../os/log_macros.h"
#include "interrupt.h"
#include "pic18f46k42.h"
static uint8_t LOG_LEVEL = L_SILENT;

/* ************************************************************************** */

void nonvolatile_memory_init(void) { log_register(); }

/* ************************************************************************** */
// NVM internal utility macros

#define SELECT_EEPROM() NVMCON1bits.REG = 0
#define SELECT_FLASH() NVMCON1bits.REG = 2

#define NVM_WRITE_MODE() NVMCON1bits.FREE = 0
#define NVM_ERASE_MODE() NVMCON1bits.FREE = 1

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

    NVMADRH = address >> 8;
    NVMADRL = address;

    SELECT_EEPROM();

    // Initiate read operation
    NVMCON1bits.RD = 1;

    // Return the value
    return NVMDAT;
}

void internal_eeprom_write(uint16_t address, uint8_t data) {
    LOG_TRACE({ println("eeprom_write"); });
    // Wait for possible previous write to complete
    if (NVMCON1bits.WR) {
        LOG_DEBUG({ println("previous write not finished"); });
        while (NVMCON1bits.WR) {
        }
    }

    NVMADRH = address >> 8;
    NVMADRL = address;

    // Load data into register
    NVMDAT = data;

    // Engage
    SELECT_EEPROM();
    NVM_WRITE_MODE();
    nvm_write();
}

/* ************************************************************************** */

/*  Notes on PIC18's Flash memory:

    Flash memory can be read as individual bytes by loading a desired address
    into TBLPTR and executing a TBLRD asm instruction.  The result of the read
    operation is placed into the TABLAT register.

    Flash memory can only be erased in 128 byte blocks. The block erase
    operation automatically handles

    Flash memory can only be written in 128 byte blocks.  In order to write only
    one or two bytes to flash without corrupting surrounding values, the
    following steps are necessary:
        1) Declare a buffer to hold 128 bytes.
        2) Call flash_read_block() with the desired address and buffer.
        3) Modify the buffer's contents with the new values.
        4) Call flash_block_write() to write the new buffer into the block.
        5) Verify that the block was written to correctly.

    Registers involved in Flash memory operations:

    NVMCON1: control bits for Flash operations
        REG:    Select between Flash/EEPROM/Configuration Bits
        n/a
        FREE:   Program Flash Memory Erase Enable bit (1)
                1 = Performs an erase operation on the next WR command
                0 = The next WR command performs a write operation
        WRERR:
        WREN:   Program/Erase Enable bit
        WR:     Write Control, set to initiate write
        RD:     Read Control, set to initiate read
    NVMCON2: receives the 'magic sequence' to allow flash writes

    TBLPTR: Table Pointer register
        A set of three registers, TBLPTRU, TBLPTRH, and TBLPTRL.
        The Microchip provided header file gives access to the three TBLPTR
        registers as a single 24 bit unsigned integer.
        These registers are used together to effectively create a 22 bit
        pointer to a byte in Program Flash Memory

    TABLAT: Table Latch register
        When reading, the memory location pointed to by the TBLPTR is copied
        into TABLAT.
        When writing, TABLAT functions as a 'shadow register' interface to
        the array of write latch holding registers. The holding registers
        are indexed by the least 7 bits of the TBLPTR.

    TBLWT and TBLRD instructions, and the POSTINC/POSTDEC/PREINC suffixes
        These instructions perform operations using the TBLPTR and TABLAT
        registers, the holding registers, and the Program Flash Memory itself.
        These operations are performed in hardware, initiated by a single
        assembly instruction. The following listings are a C-styled
        representation of the operations performed during each instruction.

        TABLAT = holdingRegister[(TBLPTR & 0x00007F)]; // "TBLRD*"
        TABLAT = holdingRegister[(TBLPTR++ & 0x00007F)]; // "TBLRD*+"
        TABLAT = holdingRegister[(TBLPTR-- & 0x00007F)]; // "TBLRD*-"
        TABLAT = holdingRegister[(TBLPTR & 0x00007F)]; // "TBLRD+*"

        holdingRegister[(TBLPTR & 0x00007F)] = TABLAT; // "TBLWT*"
        holdingRegister[(TBLPTR++ & 0x00007F)] = TABLAT; // "TBLWT*+"
        holdingRegister[(TBLPTR-- & 0x00007F)] = TABLAT; // "TBLWT*-"
        holdingRegister[(TBLPTR & 0x00007F)] = TABLAT; // "TBLWT+*"
*/

/* -------------------------------------------------------------------------- */

void print_flash_buffer(NVM_address_t address, uint8_t *buffer) {
    uint8_t element = address & FLASH_ELEMENT_MASK;

    println("");
    printf("address: %ul ", address);
    printf("block: %ul ", (address & FLASH_BLOCK_MASK));
    printf("element: %d\r\n", element);

    for (uint8_t i = 0; i < FLASH_BUFFER_SIZE; i++) {
        if (i == element) {
            printf("\033[7m%02x\033[0;37;40m ", buffer[i]);
        } else {
            printf("%02x ", buffer[i]);
        }
        if (((i + 1) % 16) == 0) {
            println("");
        }
    }
    println("");
}

static void set_TBLPTR(NVM_address_t address) {
    LOG_DEBUG({ printf("address: %ul\r\n", address); });
    TBLPTR = address;
    LOG_DEBUG({ printf("TBLPTR: %ul\r\n", TBLPTR); });
}

/* -------------------------------------------------------------------------- */

// Read one byte from Flash memory at (address)
uint8_t flash_read_byte(NVM_address_t address) {
    LOG_TRACE({ println("flash_read_byte"); });

    set_TBLPTR(address);

    // Read one byte from flash to TABLAT
    SELECT_FLASH();
    asm("TBLRD");
    return TABLAT;
}

// Write one byte into flash memory at (address)
void flash_write_byte(NVM_address_t address, uint8_t data) {
    LOG_TRACE({ println("flash_write_byte"); });
    SELECT_FLASH();

    // if existingData is already what we want, then we're done
    uint8_t existingData = flash_read_byte(address);
    if (existingData == data) {
        return;
    }

    // compare the two datas bit-by-bit, checking for 0->1 transitions
    bool mustErase = false;
    for (uint8_t i = 0; i < 8; i++) {
        if (((existingData & (1 << i)) == 0) && ((data & (1 << i)) == 1)) {
            mustErase = true;
        }
    }

    // Read existing block into buffer
    uint8_t buffer[FLASH_BUFFER_SIZE];
    flash_read_block(address, buffer);

    // write the new data into the buffer
    buffer[address & FLASH_ELEMENT_MASK] = data;

    // only erase if we require a 0 -> 1 transition
    if (mustErase) {
        flash_block_erase(address);
    }

    // Write the modified buffer back into flash
    flash_block_write(address, buffer);
}

// Read an entire block of 64 bytes from Flash memory into the provided buffer
void flash_read_block(NVM_address_t address, uint8_t *readBuffer) {
    LOG_TRACE({ println("flash_read_block"); });

    set_TBLPTR(address & FLASH_BLOCK_MASK);

    // copy TABLAT into readBuffer
    SELECT_FLASH();
    for (uint8_t i = 0; i < FLASH_BUFFER_SIZE; i++) {
        asm("TBLRDPOSTINC");
        readBuffer[i] = TABLAT;
    }
}

// Erase a block of Flash memory at (address)
void flash_block_erase(NVM_address_t address) {
    LOG_TRACE({ println("flash_block_erase"); });

    set_TBLPTR(address);

    // Engage
    SELECT_FLASH();
    NVM_ERASE_MODE();
    nvm_write();
}

// Write the provided buffer into flash memory at (address)
void flash_block_write(NVM_address_t address, uint8_t *writeBuffer) {
    LOG_TRACE({ println("flash_block_write"); });

    set_TBLPTR(address & FLASH_BLOCK_MASK);

    // copy writeBuffer into TABLAT
    for (uint8_t i = 0; i < FLASH_BUFFER_SIZE; i++) {
        TABLAT = writeBuffer[i]; // Load data byte
        if (i == (FLASH_BUFFER_SIZE - 1)) {
            asm("TBLWT");
        } else {
            asm("TBLWTPOSTINC");
        }
    }

    // Engage
    SELECT_FLASH();
    NVM_WRITE_MODE();
    nvm_write();
}

/* ************************************************************************** */

// Diagnostic shell commands for nonvolatile_memory driver

#if 0

/* -------------------------------------------------------------------------- */

int shell_eeprom(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \teeprom write <address> <data>");
        println("\teeprom read <address>");
        return 0;

    case 3:
        if (!strcmp(argv[1], "read")) {
            LOG_TRACE({ println("eeprom read <address>"); });
            uint16_t address = atoi(argv[2]);
            LOG_DEBUG({ printf("address: %ul\r\n", (uint32_t)address); });

            printf("%02x\r\n", internal_eeprom_read(address));
            return 0;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            LOG_TRACE({ println("eeprom write <address> <data>"); });

            uint16_t address = atoi(argv[2]);
            uint8_t data = atoi(argv[3]);

            LOG_DEBUG({ printf("address: %ul\r\n", (uint32_t)address); });

            internal_eeprom_write(address, data);
            printf("%02x\r\n", internal_eeprom_read(address));
            return 0;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

/* -------------------------------------------------------------------------- */

NVM_address_t decode_address(char *string) {
    uint8_t length = strlen(string);

    bool decimal = true;

    for (uint8_t i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            decimal = false;
        }
    }
    if (decimal) {
        int32_t temp = atol(string);
        if ((temp >= 0) && (temp < FLASH_SIZE)) {
            return (NVM_address_t)temp;
        }
    }
    return 0xffffff;
}

int16_t decode_data(char *string) {
    uint8_t length = strlen(string);

    bool decimal = true;
    bool hex = true;

    for (uint8_t i = 0; i < length; i++) {
        if (!isdigit(string[i])) {
            decimal = false;
        }
    }
    if (decimal) {
        return atoi(string);
    }

    for (uint8_t i = 0; i < length; i++) {
        if (!isxdigit(string[i])) {
            hex = false;
        }
    }
    if (hex) {
        return xtoi(string);
    }
    return -1;
}

int shell_flash(int argc, char **argv) {
    switch (argc) {
    case 1:
        println("usage: \tflash write <address> <data>");
        println("\tflash read <address>");
        println("\tflash bread <address>");
        println("\tValid addresses are between 0 and 65355:");
        println("\tData must be a single byte in decimal or hex format");
        return 0;

    case 3:
        if (!strcmp(argv[1], "read")) {
            LOG_TRACE({ println("flash read <address>"); });

            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return 0;
            }

            printf("%02x\r\n", flash_read_byte(address));
            return 0;
        } else if (!strcmp(argv[1], "bread")) {
            LOG_TRACE({ println("flash bread <address>"); });

            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return 0;
            }

            // Read existing block into buffer
            uint8_t buffer[FLASH_BUFFER_SIZE];
            flash_read_block(address, buffer);
            print_flash_buffer(address, buffer);
            return 0;
        }
        break;

    case 4:
        if (!strcmp(argv[1], "write")) {
            LOG_TRACE({ println("flash write <address> <data>"); });

            // parse address
            NVM_address_t address = decode_address(argv[2]);
            if (address == 0xffffff) {
                println("invalid address");
                return 0;
            }

            // parse data
            int16_t test = decode_data(argv[3]);
            if (test < 0 || test > 255) {
                println("invalid data");
                return 0;
            }
            uint8_t newData = (uint8_t)test;

            // return if the address already contains the data we want
            uint8_t existingData = flash_read_byte(address);
            if (existingData == newData) {
                return 0;
            }

            LOG_DEBUG({
                printf("existingData: '%u', newData: '%u'\r\n",
                       (uint16_t)existingData, (uint16_t)newData);
            });
            // compare the two datas bit-by-bit, checking for 0->1 transitions
            bool mustErase = false;
            for (uint8_t i = 0; i < 8; i++) {
                if (!(existingData & (1 << i)) && (newData & (1 << i))) {
                    mustErase = true;
                }
            }

            // Read existing block into buffer
            uint8_t buffer[FLASH_BUFFER_SIZE];
            flash_read_block(address, buffer);
            LOG_DEBUG({
                println("existing data:");
                print_flash_buffer(address, buffer);
            });

            // write newData into the buffer
            buffer[address & FLASH_ELEMENT_MASK] = newData;

            // only erase if we need to
            if (mustErase) {
                LOG_DEBUG({ println("erasing"); });
                flash_block_erase(address);
            }

            // Write the modified buffer back into flash
            LOG_DEBUG({ println("writing"); });
            flash_block_write(address, buffer);

            // Verify that the write worked
            flash_read_block(address, buffer);
            LOG_DEBUG({
                println("verifying:");
                print_flash_buffer(address, buffer);
            });

            return 0;
        }
        break;

    default:
        break;
    }
    println("invalid arguments");
    return 0;
}

#endif