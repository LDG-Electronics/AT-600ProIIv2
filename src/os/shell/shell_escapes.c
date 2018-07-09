#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

typedef struct {
    char buffer[ESCAPE_BUFFER_LENGTH];
    uint8_t length;
} escape_t;

escape_t escape;

void reset_escape_buffer(void) {
    for (uint8_t i = 0; i < ESCAPE_BUFFER_LENGTH; i++) {
        escape.buffer[i] = 0;
    }
    escape.length = 1;
}

/* -------------------------------------------------------------------------- */

// key name debugging

void print_key_name(const char *string) {
    if (shell.keyNameDebugMode == 1) {
        println(string);
    }
}

void toggle_echo_key_name_debug_mode(void) {
    shell.keyNameDebugMode = !shell.keyNameDebugMode;
    if (shell.keyNameDebugMode == 1) {
        println("Now printing key names.");
    } else {
        println("No longer printing key names");
        print(SHELL_PROMPT_STRING);
    }
}

/* -------------------------------------------------------------------------- */

/*  raw echo mode is a shell mode that, instead of printing characters normally,
    only echos back the decimal values of received UART bytes, and attempts to
    sort them into groups that represent escape sequences.

    This mode is used to build escape sequence charts and to assist in
    diagnosing issues and failures when processing non-printable keypresses.

*/
void toggle_raw_echo_mode(void) {
    shell.rawEchoMode = !shell.rawEchoMode;
    if (shell.rawEchoMode == 1) {
        println("Entering Raw Echo Mode.");
    } else {
        println("Leaving Raw Echo Mode.");
        print(SHELL_PROMPT_STRING);
    }
}

/* -------------------------------------------------------------------------- */

/*  Notes on escape sequences:

    If a block returns, that means that the escape sequence isn't over yet.

    If a block ends with "goto FINISHED;", that means that we know it was a
    valid escape sequence.
*/
void process_escape_sequence(char currentChar) {
    if (shell.escapeMode == 0) {
        shell.escapeMode = 1;
        reset_escape_buffer();
        return;
    }

    char prevChar = escape.buffer[escape.length];
    escape.length++;
    escape.buffer[escape.length] = currentChar;

    switch (escape.length) {
    case 0:
    case 1:
        return; // fallthrough
    case 2:
        switch (currentChar) {
        case KEY_ALT_BS: // delete all characters to the left of the cursor
            print_key_name("ctrl + backspace");
            while (shell.cursor > 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            goto FINISHED;
        }

    case 3:
        switch (currentChar) {
        case KEY_UP:
            print_key_name("up");
            goto FINISHED;
        case KEY_DOWN:
            print_key_name("down");
            goto FINISHED;
        case KEY_RIGHT:
            print_key_name("right");
            move_cursor(1);
            goto FINISHED;
        case KEY_LEFT:
            print_key_name("left");
            move_cursor(-1);
            goto FINISHED;
        case KEY_HOME:
            print_key_name("home");
            move_cursor_to(0);
            goto FINISHED;
        case KEY_END:
            print_key_name("end");
            move_cursor_to(shell.length);
            goto FINISHED;
        case KEY_F1:
            print_key_name("F1");
            goto FINISHED;
        case KEY_F2:
            print_key_name("F2");
            goto FINISHED;
        case KEY_F3:
            print_key_name("F3");
            goto FINISHED;
        case KEY_F4:
            print_key_name("F4");
            goto FINISHED;
        }
        return;
    case 4:
        switch (currentChar) {
        case '~':
            switch (prevChar) {
            case KEY_PGUP:
                print_key_name("pgup");
                goto FINISHED;
            case KEY_PGDN:
                print_key_name("pgdn");
                goto FINISHED;
            case KEY_DEL:
                print_key_name("delete");
                if (shell.cursor != shell.length) {
                    remove_char_at_cursor();
                }
                goto FINISHED;
            case KEY_INS:
                print_key_name("insert");
                goto FINISHED;
            }
        }
        return;
    case 5:
        if (currentChar == '~') {
            switch (prevChar) {
            case KEY_F5:
                print_key_name("F5");
                goto FINISHED;
            case KEY_F6:
                print_key_name("F6");
                goto FINISHED;
            case KEY_F7:
                print_key_name("F7");
                goto FINISHED;
            case KEY_F8:
                print_key_name("F8");
                goto FINISHED;
            case KEY_F9:
                print_key_name("F9");
                toggle_raw_echo_mode();
                goto FINISHED;
            case KEY_F10:
                print_key_name("F10");
                toggle_echo_key_name_debug_mode();
                goto FINISHED;
            case KEY_F11:
                print_key_name("F11");
                goto FINISHED;
            case KEY_F12:
                print_key_name("F12");
                goto FINISHED;
            }
        }
        return;
    case 6:
        switch (currentChar) {
        case KEY_RIGHT:
            switch (escape.buffer[5]) {
            case MOD_SHIFT:
                print_key_name("shift + right");
                goto FINISHED;
            case MOD_ALT:
                print_key_name("alt + right");
                goto FINISHED;
            case MOD_CTRL:
                print_key_name("ctrl + right");
                goto FINISHED;
            }
        case KEY_LEFT:
            switch (escape.buffer[5]) {
            case MOD_SHIFT:
                print_key_name("shift + left");
                goto FINISHED;
            case MOD_ALT:
                print_key_name("alt + left");
                goto FINISHED;
            case MOD_CTRL:
                print_key_name("ctrl + left");
                goto FINISHED;
            }
        case KEY_HOME:
            switch (escape.buffer[5]) {
            case MOD_SHIFT:
                print_key_name("shift + home");
                goto FINISHED;
            case MOD_ALT:
                print_key_name("alt + home");
                goto FINISHED;
            case MOD_CTRL:
                print_key_name("ctrl + home");
                goto FINISHED;
            }
        case KEY_END:
            switch (escape.buffer[5]) {
            case MOD_SHIFT:
                print_key_name("shift + end");
                goto FINISHED;
            case MOD_ALT:
                print_key_name("alt + end");
                goto FINISHED;
            case MOD_CTRL:
                print_key_name("ctrl + end");
                goto FINISHED;
            }
        case '~':
            switch (escape.buffer[3]) {
            case KEY_DEL:
                switch (escape.buffer[5]) {
                case MOD_SHIFT:
                    print_key_name("shift + delete");
                    goto FINISHED;
                case MOD_ALT:
                    print_key_name("alt + delete");
                    goto FINISHED;
                case MOD_CTRL:
                    print_key_name("ctrl + delete");
                    goto FINISHED;
                }
            case KEY_INS:
                switch (escape.buffer[5]) {
                case MOD_SHIFT:
                    print_key_name("shift + insert");
                    goto FINISHED;
                case MOD_ALT:
                    print_key_name("alt + insert");
                    goto FINISHED;
                case MOD_CTRL:
                    print_key_name("ctrl + insert");
                    goto FINISHED;
                }
            }
        }
        return;
    case 7:
        goto FINISHED;
    }

FINISHED:
    // if we're in raw echo mode, then add a newline to seperate groups of
    // escape sequence codes
    if (shell.rawEchoMode) {
        printf(" length: %d\r\n", escape.length);
        println("");
    }

    // If we reach this spot, then we've fully processed the current
    // escape sequence, and we can exit escape mode.
    reset_escape_buffer();
    shell.escapeMode = 0;
}