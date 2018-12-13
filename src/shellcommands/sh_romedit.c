#include "../os/serial_port.h"
#include "../os/shell/shell.h"
#include "../os/shell/shell_cursor.h"
#include "../os/shell/shell_keys.h"
#include "../os/shell/shell_utils.h"
#include "../peripherals/nonvolatile_memory.h"
#include "sh_flash.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

NVM_address_t currentAddress;

shell_line_t lineBuffer;

/* -------------------------------------------------------------------------- */

void draw_romedit_prompt(void) {
    term_cursor_set(0, 0);
    term_cursor_down(7);
    print("\033[31m");
    print("> ");
    reset_text_attributes();
}

/* -------------------------------------------------------------------------- */

int8_t romedit_process_command(char *string) {
    int argc = 0;
    char *argv[CONFIG_SHELL_MAX_COMMAND_ARGS];

    // tokenize the shell buffer
    // argv_list will end up containing a pointer to each token
    char *token = strtok(string, " ");
    while (token != NULL && argc < CONFIG_SHELL_MAX_COMMAND_ARGS) {
        argv[argc++] = token;
        token = strtok(NULL, " ");
    }

    switch (argc) {
    case 1:
        if (!strcmp(argv[0], "help")) {
            println("");
            println("usage: \t write <data>");
            println("\t goto <address>");
            println("\t exit");
            return 0;
        }
        if (!strcmp(argv[0], "exit")) {
            return -1;
        }
        break;
    case 2:
        if (!strcmp(argv[0], "write")) {
            // parse data
            int16_t data = decode_data(argv[1]);
            if (data < 0 || data > 255) {
                println("invalid data");
                return 0;
            }

            write_single_byte(currentAddress, data);
            return 0;
        }

        if (!strcmp(argv[0], "goto")) {
            // parse address
            NVM_address_t address = decode_address(argv[1]);
            if (address == 0xffffff || address < 192 || address > FLASH_SIZE) {
                println("invalid address");
                return 0;
            }

            currentAddress = address;

            term_cursor_set(0, 0);
            print_flash_block(currentAddress);
            println("");
            draw_romedit_prompt();
            move_cursor_to(&lineBuffer, lineBuffer.cursor);

            return 0;
        }
        break;
    }

    println("");
    printf("%s: command not found\r\n", lineBuffer.buffer);
    return 0;
}

int8_t romedit_keys(key_t key) {
    switch (key.key) {
    default:
        return 0;
    case BACKSPACE:
        if (lineBuffer.cursor != 0) {
            move_cursor_left(&lineBuffer);
            remove_char_at_cursor(&lineBuffer);
        }
        return 0;
    case UP:
        if ((currentAddress - 16) >= 192) {
            currentAddress -= 16;
        } else {
            return 0;
        }
        break;
    case DOWN:
        if ((currentAddress + 16) <= (FLASH_SIZE - 1)) {
            currentAddress += 16;
        } else {
            return 0;
        }
        break;
    case LEFT:
        if ((currentAddress - 1) >= 192) {
            currentAddress -= 1;
        } else {
            return 0;
        }
        break;
    case RIGHT:
        if ((currentAddress + 1) <= (FLASH_SIZE - 1)) {
            currentAddress += 1;
        } else {
            return 0;
        }
        break;
    case PAGEUP:
        if ((currentAddress - 128) >= 192) {
            currentAddress -= 128;
        } else {
            return 0;
        }
        break;
    case PAGEDOWN:
        if ((currentAddress + 128) <= (FLASH_SIZE - 1)) {
            currentAddress += 128;
        } else {
            return 0;
        }
        break;
    case F5:
        term_reset_screen();
        break;
    case ENTER:
        if (lineBuffer.length > 0) {
            // add terminating null to shell buffer
            lineBuffer.buffer[lineBuffer.length] = '\0';

            term_cursor_home();
            term_cursor_down(1);
            term_clear_to_right();
            term_cursor_down(1);
            term_clear_to_right();
            term_cursor_down(1);
            term_clear_to_right();
            term_cursor_set(8, 3);

            // attempt to process the shell buffer
            if (romedit_process_command(&lineBuffer.buffer[0]) == -1) {
                return -1;
            }

            // wipe the current shell line
            memset(&lineBuffer, 0, sizeof(shell_line_t));

            // move the cursor to right after the prompt
            term_cursor_set(8, 3);

            // ...for us to delete the line from the terminal
            term_clear_to_right();

            // move the cursor to right after the prompt
            term_cursor_set(8, 3);
        }
        return 0;
    case ESCAPE:
        return -1;
    }

    term_cursor_set(0, 0);
    print_flash_block(currentAddress);
    println("");
    draw_romedit_prompt();
    move_cursor_to(&lineBuffer, lineBuffer.cursor);

    return 0;
}

/* **************************************************************************
 */

int8_t romedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        return romedit_keys(key);
    }

    if (isprint(currentChar)) {
        insert_char_at_cursor(&lineBuffer, currentChar);
        return 0;
    }
    return 0;
}

// setup
void romedit(int argc, char **argv) {
    switch (argc) {
    case 1:
        currentAddress = 192;
        break;
    case 2:
        currentAddress = decode_address(argv[1]);
        if (currentAddress == 0xffffff) {
            println("invalid address");
            return;
        }
        break;
    default:
        return;
    }

    memset(&lineBuffer, 0, sizeof(shell_line_t));

    term_reset_screen();
    term_cursor_set(0, 0);
    print_flash_block(currentAddress);
    println("");
    draw_romedit_prompt();

    shell_register_callback(romedit_callback);
}