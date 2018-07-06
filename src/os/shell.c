#include "../includes.h"
#include "shell_keycodes.h"

/* ************************************************************************** */

struct shell_command_entry {
    shell_program_t shell_program;
    const char *shell_command_string;
};

struct shell_command_entry list[CONFIG_SHELL_MAX_COMMANDS];

/* -------------------------------------------------------------------------- */

typedef struct {
    shell_program_t callback;
    const char *command;
} shell_entry_t;

typedef struct commands {
    shell_entry_t list[CONFIG_SHELL_MAX_COMMANDS];
    uint8_t numOfRegisteredCommands;
} command_list_t;

command_list_t commands;

void init_shell_commands(void) {
    for (uint8_t i = 0; i < SHELL_BUFFER_LENGTH; i++) {
        commands.list[i].callback = NULL;
        commands.list[i].command = NULL;
    }
    commands.numOfRegisteredCommands = 0;
}

/* -------------------------------------------------------------------------- */

char shell_rx_buffer[CONFIG_SHELL_MAX_INPUT];

/* -------------------------------------------------------------------------- */

typedef struct {
    char buffer[SHELL_BUFFER_LENGTH];
    uint8_t length;
    uint8_t cursorLocation;
} shell_buffer_t;

shell_buffer_t shellBuffer;

void clear_shell_buffer(shell_buffer_t *ptr) {
    for (uint8_t i = 0; i < SHELL_BUFFER_LENGTH; i++) {
        ptr->buffer[i] = 0;
    }
    ptr->length = 0;
    ptr->cursorLocation = 0;
}

/* ************************************************************************** */

void shell_init(void) {
    for (uint8_t i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        list[i].shell_program = 0;
        list[i].shell_command_string = 0;
    }

    init_shell_commands();
    clear_shell_buffer(&shellBuffer);

    println(SHELL_VERSION_STRING);
    print(SHELL_PROMPT_STRING);
}

bool shell_register(shell_program_t program, const char *string) {
    unsigned char i;

    for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        if (list[i].shell_program != 0 || list[i].shell_command_string != 0)
            continue;
        list[i].shell_program = program;
        list[i].shell_command_string = string;
        return true;
    }
    return false;
}

/* -------------------------------------------------------------------------- */

void process_shell_command(void) {
    int argc = 0;
    int length = strlen(shell_rx_buffer) + 1;
    char toggle = 0;

	char *argv_list[CONFIG_SHELL_MAX_COMMAND_ARGS];

    argv_list[argc] = &shell_rx_buffer[0];

    for (uint8_t i = 0; i < length && argc < CONFIG_SHELL_MAX_COMMAND_ARGS; i++) {
        switch (shell_rx_buffer[i]) {
        case '\0': // String terminator means at least one arg
            i = length;
            argc++;
            break;
        case '\"': // Check for double quotes for strings as parameters
            if (toggle == 0) {
                toggle = 1;
                shell_rx_buffer[i] = '\0';
                argv_list[argc] = &shell_rx_buffer[i + 1];
            } else {
                toggle = 0;
                shell_rx_buffer[i] = '\0';
            }
            break;
        case ' ': // Command arguments are separated by spaces
            if (toggle == 0) {
                shell_rx_buffer[i] = '\0';
                argc++;
                argv_list[argc] = &shell_rx_buffer[i + 1];
            }
            break;
        }
    }

	int retval = 0;
    // sequential search on command table
    for (uint8_t i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        if (list[i].shell_program == 0)
            continue;

        // If string matches one on the list
        if (!strcmp(argv_list[0], list[i].shell_command_string)) {
            // Run the appropiate function
            retval = list[i].shell_program(argc, argv_list);

            // check retval for... what?

            return;
        }
    }
    printf("No command '%s' found.", argv_list[0]);
}

void shell_update(void) {
    static char prevChar = 0;

    char currentChar = getch();

    // return early if we haven't rx'd a character
    if (currentChar == KEY_NUL)
        return;

    if (prevChar == KEY_ESC) {
        if (currentChar == '[') {
            return;
        }
    }

    if (prevChar == '[') {
        if (currentChar == KEY_UP) {
            println("UP");
            return;
        }
        if (currentChar == KEY_DOWN) {
            println("DOWN");
            return;
        }
        if (currentChar == KEY_RIGHT) {
            println("RIGHT");
            return;
        }
        if (currentChar == KEY_LEFT) {
            // putch(KEY_ESC);
            // putch(KEY_BS);
            print("\[1D");
            return;
        }
    }

    prevChar = currentChar;

    // Number of characters written to buffer (this should be static var)
    static unsigned short count = 0;

    // process control characters
    if (iscntrl(currentChar)) {
        if (currentChar == KEY_ESC) {
            // println(&currentChar);
            return;
        }

        if (currentChar == SHELL_ASCII_DEL) {
            // println("del");
            return;
        }

        if (currentChar == SHELL_ASCII_HT) {
            // println("tab");
            return;
        }

        if (currentChar == KEY_BS) {
            if (count > 0) {
                count--;
                putch(KEY_BS);
                putch(KEY_SP);
                putch(KEY_BS);
            } else {
                putch(SHELL_ASCII_BEL);
            }
            return;
        }

        if (currentChar == SHELL_ASCII_CR) {
            shell_rx_buffer[count] = '\0';
            println("");
            if (count > 0) {
                process_shell_command();

                count = 0;
            } else {
                println("command not found");
            }
            println("");
            print(SHELL_PROMPT_STRING);
            return;
        }
    }

    // process printable characters
    if (isprint(currentChar)) {
        if (count < CONFIG_SHELL_MAX_INPUT) {
            shell_rx_buffer[count] = currentChar;
            putch(currentChar);
            count++;
        }
        return;
    }
}

/* -------------------------------------------------------------------------- */

void shell_print_commands(void) {
    unsigned char i;

    for (i = 0; i < CONFIG_SHELL_MAX_COMMANDS; i++) {
        if (list[i].shell_program != 0 || list[i].shell_command_string != 0) {
            println(list[i].shell_command_string);
        }
    }
}
