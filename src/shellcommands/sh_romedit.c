#ifdef DEVELOPMENT

/* ************************************************************************** */

#include "os/shell/shell_command_processor.h"
#include "os/shell/shell_command_utils.h"
#include "os/shell/shell_cursor.h"
#include "os/shell/shell_keys.h"
#include "os/shell/shell_utils.h"
#include "peripherals/nonvolatile_memory.h"
#include <stdbool.h>

extern void write_single_byte(NVM_address_t address, uint8_t newData);

/* ************************************************************************** */

static NVM_address_t decode_address(char *string) {
    bool decimal = true;

    uint8_t i = 0;
    while (string[i]) {
        if (!isdigit(string[i++])) {
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

static int16_t decode_data(char *string) {
    bool decimal = true;
    bool hex = true;

    uint8_t i = 0;
    while (string[i]) {
        if (!isdigit(string[i++])) {
            decimal = false;
        }
    }
    if (decimal) {
        return atoi(string);
    }

    i = 0;
    while (string[i]) {
        if (!isxdigit(string[i++])) {
            hex = false;
        }
    }
    if (hex) {
        return strtol(string, NULL, 16);
    }
    return -1;
}

/* ************************************************************************** */

typedef struct {
    NVM_address_t address;
    shell_line_t line;
} romedit_state_t;

romedit_state_t state;

/* -------------------------------------------------------------------------- */

#define move_cursor_to_prompt() term_cursor_set(3, 15)

void draw_romedit_prompt(void) {
    term_cursor_set(0, 15);
    print("\033[1;31;40m");
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

void draw_grid_outline(void) {
    term_cursor_set(0, 0);
    println("-----------------------------------------------------");
    println("|  address  |  blockAddress  |  block#  |  element  |");

    term_cursor_set(0, 4);
    println("-----------------------------------------------------");

    term_cursor_set(0, 13);
    println("-----------------------------------------------------");
}

void draw_grid_contents(NVM_address_t address) {
    uint8_t element = address & FLASH_ELEMENT_MASK;
    NVM_address_t blockAddress = address & FLASH_BLOCK_MASK;
    uint16_t blockNumber = blockAddress / 128;

    term_cursor_set(0, 3);
    printf("|  %-7lu  |  %-12lu  |  %-6u  |  %-7u  |\r\n", (uint32_t)address,
           (uint32_t)blockAddress, blockNumber, element);

    // create a pointer to the first byte of the selected block
    const char *blockPointer = (const char *)blockAddress;

    term_cursor_set(0, 5);
    print("|  "); // start of first row
    for (uint8_t i = 0; i < FLASH_BUFFER_SIZE; i++) {
        char tempChar = *blockPointer++;
        if (i == element) {
            printf("\033[7m%02x\033[0;37;40m ", tempChar);
        } else {
            printf("%02x ", tempChar);
        }
        if (((i + 1) % 16) == 0) {
            println(" |"); // end of current row
            if (i < FLASH_BUFFER_SIZE - 1) {
                print("|  "); // start of next row
            }
        }
    }
}

// redraw the upper half of the screen
void refresh_grid(void) {
    term_hide_cursor();
    draw_grid_contents(state.address);
}

// redraw the lower half
void refresh_shell(void) {
    draw_romedit_prompt();
    draw_line(&state.line);
    term_show_cursor();
}

/* -------------------------------------------------------------------------- */

int8_t romedit_process_command(shell_line_t *line) {
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

            write_single_byte(state.address, data);
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
            if (address == 0xffffff || address < 256 || address > FLASH_SIZE) {
                println("invalid address");
                return 0;
            }

            state.address = address;

            refresh_grid();
            refresh_shell();

            return 0;
        } else {
            println("");
            println("invalid arguments");
            return 0;
        }
    }

    println("");
    printf("%s: command not found\r\n", state.line.buffer);
    return 0;
}

/* -------------------------------------------------------------------------- */

bool increment_address(uint16_t distance) {
    if ((state.address + distance) <= (FLASH_SIZE - 1)) {
        state.address += distance;
        return true;
    }
    return false;
}

bool decrement_address(uint16_t distance) {
    if ((state.address - distance) >= 256) {
        state.address -= distance;
        return true;
    }
    return false;
}

int8_t romedit_keys(key_t key) {
    switch (key.key) {
    default:
        return 0;
    case BACKSPACE:
        if (state.line.cursor != 0) {
            move_cursor_left(&state.line);
            remove_char_at_cursor(&state.line);
        }
        return 0;
    case UP:
        if (decrement_address(16)) {
            break;
        }
        return 0;
    case DOWN:
        if (increment_address(16)) {
            break;
        }
        return 0;
    case LEFT:
        if (decrement_address(1)) {
            break;
        }
        return 0;
    case RIGHT:
        if (increment_address(1)) {
            break;
        }
        return 0;
    case PAGEUP:
        if (decrement_address(128)) {
            break;
        }
        return 0;
    case PAGEDOWN:
        if (increment_address(128)) {
            break;
        }
        return 0;
    case F5:
        term_reset_screen();
        draw_grid_outline();
        break;
    case ENTER:
        if (state.line.length > 0) {
            shell_add_terminator_to_line(state.line);

            // remove any output from previous commands
            erase_below_prompt();

            if (romedit_process_command(&state.line) == -1) {
                return -1;
            }

            shell_reset_line(state.line);
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

/* ************************************************************************** */

// loop
int8_t romedit_callback(char currentChar) {
    if (iscntrl(currentChar)) {
        key_t key = identify_key(currentChar);
        return romedit_keys(key);
    }

    if (isprint(currentChar)) {
        insert_char_at_cursor(&state.line, currentChar);
        return 0;
    }
    return 0;
}

// setup
void sh_romedit(int argc, char **argv) {
    switch (argc) {
    case 1:
        state.address = 256;
        break;
    case 2:
        state.address = decode_address(argv[1]);
        if (state.address == 0xffffff) {
            println("invalid address");
            return;
        }
        break;
    default:
        return;
    }

    shell_reset_line(state.line);

    term_reset_screen();

    draw_grid_outline();
    refresh_grid();
    refresh_shell();

    shell_register_callback(romedit_callback);
}

REGISTER_SHELL_COMMAND(sh_romedit, "romedit");

#endif // #ifdef DEVELOPMENT