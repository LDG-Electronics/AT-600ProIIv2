#include "../os/serial_port.h"
#include "../os/shell/shell.h"
#include "../os/shell/shell_command_processor.h"
#include "../os/shell/shell_cursor.h"
#include "../os/shell/shell_keys.h"
#include "../os/shell/shell_utils.h"
#include "../peripherals/nonvolatile_memory.h"
#include "sh_flash.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

typedef struct {
    NVM_address_t address;
    shell_line_t line;
} romedit_t;

romedit_t re;

NVM_address_t currentAddress;
shell_line_t lineBuffer;

/* -------------------------------------------------------------------------- */

#define move_cursor_to_prompt() term_cursor_set(8, 3)

void draw_romedit_prompt(void) {
    term_cursor_set(0, 0);
    term_cursor_down(7);
    print("\033[31m");
    print("> ");
    reset_text_attributes();
}

void erase_below_prompt(void) {
    term_cursor_home();
    for (size_t i = 0; i < 5; i++) {
        term_cursor_down(1);
        term_clear_to_right();
    }
    move_cursor_to_prompt();
}

/* -------------------------------------------------------------------------- */

// redraw the upper half of the screen
void refresh_grid(void) {
    term_cursor_set(0, 0);
    print_flash_block(currentAddress);
}

// redraw the lower halfd
void refresh_shell(void) {
    draw_romedit_prompt();
    draw_line(&lineBuffer);
}

/* -------------------------------------------------------------------------- */

int8_t romedit_process_command(shell_line_t *line) {
    // parse or line into args
    shell_args_t args = parse_shell_line(line);

    // help command
    if (!strcmp(args.argv[0], "help")) {
        println("");
        println("usage: \t write <data>");
        println("\t goto <address>");
        println("\t exit");
        return 0;
    }

    // exit command
    if (!strcmp(args.argv[0], "exit")) {
        return -1;
    }

    // write command
    if (!strcmp(args.argv[0], "write")) {
        if (args.argc == 2) {
            // parse data
            int16_t data = decode_data(args.argv[1]);
            if (data < 0 || data > 255) {
                println("invalid data");
                return 0;
            }

            write_single_byte(currentAddress, data);
            return 0;
        } else {
            println("");
            println("invalid arguments");
            return 0;
        }
    }

    // goto command
    if (!strcmp(args.argv[0], "goto")) {
        if (args.argc == 2) {
            // parse address
            NVM_address_t address = decode_address(args.argv[1]);
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
        } else {
            println("");
            println("invalid arguments");
            return 0;
        }
    }

    println("");
    printf("%s: command not found\r\n", lineBuffer.buffer);
    return 0;
}

/* -------------------------------------------------------------------------- */

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
            shell_add_terminator_to_line(lineBuffer);

            // remove any output from previous commands
            erase_below_prompt();

            if (romedit_process_command(&lineBuffer) == -1) {
                return -1;
            }

            shell_reset_line(lineBuffer);
            refresh_shell();
        }
        return 0;
    case ESCAPE:
        return -1;
    }

    refresh_grid();
    refresh_shell();

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

    shell_reset_line(lineBuffer);

    term_reset_screen();

    refresh_grid();
    refresh_shell();

    shell_register_callback(romedit_callback);
}