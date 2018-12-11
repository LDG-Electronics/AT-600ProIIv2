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

/* -------------------------------------------------------------------------- */

shell_line_t lineBuffer;

void draw_romedit_prompt(void) {
    term_cursor_set(0, 0);
    print("\033[31m");
    print("> ");
    reset_text_attributes();
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
        currentAddress -= 16;
        break;
    case DOWN:
        currentAddress += 16;
        break;
    case LEFT:
        currentAddress -= 1;
        break;
    case RIGHT:
        currentAddress += 1;
        break;
    case PAGEUP:
        currentAddress -= 128;
        break;
    case PAGEDOWN:
        currentAddress += 128;
        break;
    case F5:
        break;
    case ENTER:
        if (lineBuffer.length > 0) {
            memset(&lineBuffer, 0, sizeof(shell_line_t));
            draw_line(&lineBuffer);
        }
        return 0;
    case ESCAPE:
        return -1;
    }

    print_flash_block(currentAddress);
    draw_romedit_prompt();
    move_cursor_to(&lineBuffer, lineBuffer.cursor);

    return 0;
}

/* ************************************************************************** */

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
        currentAddress = 0;
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
    print_flash_block(currentAddress);
    draw_romedit_prompt();

    shell_register_callback(romedit_callback);
}