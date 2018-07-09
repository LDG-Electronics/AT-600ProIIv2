#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

typedef struct {
    char buffer[SEQUENCE_BUFFER_LENGTH];
    uint8_t length;
} sequence_t;

sequence_t sequence;

void reset_sequence_buffer(void) {
    for (uint8_t i = 0; i < SEQUENCE_BUFFER_LENGTH; i++) {
        sequence.buffer[i] = 0;
    }
    sequence.length = 0;
}

/* -------------------------------------------------------------------------- */

// key name debugging
char keyName[32];

void set_key_name(const char *string) {
    if (shell.sequenceInspectionMode == 1) {
        uint8_t i = 0;
        while (string[i]) {
            keyName[i] = string[i];
            i++;
        }
        keyName[i] = NULL;
    }
}

void print_key_name(void) {
    print("\'");
    print((const char *)keyName);
    print("\'");
}

/* -------------------------------------------------------------------------- */

/*  sequence inspection mode is a shell mode that, instead of printing
    characters normally, only echos back the decimal values of received UART
    bytes, and attempts to sort them into groups that represent escape
    sequences.

    This mode is used to build escape sequence charts and to assist in
    diagnosing issues and failures when processing non-printable keypresses.

*/
void toggle_sequence_inspection_mode(void) {
    shell.sequenceInspectionMode = !shell.sequenceInspectionMode;
    if (shell.sequenceInspectionMode == 1) {
        println("\r\nSequence inspection mode enabled.");
    } else {
        println("\r\nSequence inspection mode disabled.");
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
        reset_sequence_buffer();
    }

    char prevChar = sequence.buffer[sequence.length];
    sequence.length++;
    sequence.buffer[sequence.length] = currentChar;

    switch (sequence.length) {
    case 1:
        switch (currentChar) {
        case KEY_CTRL_C:
            set_key_name("ctrl + c");
            goto FINISHED;
        case KEY_CTRL_D: // delete one character to the right of the cursor
            set_key_name("ctrl + d");
            if (shell.cursor != shell.length) {
                remove_char_at_cursor();
            }
            goto FINISHED;
        case KEY_CTRL_E: // move cursor to the end of the line
            set_key_name("ctrl + e");
            move_cursor_to(shell.length);
            goto FINISHED;
        case KEY_CTRL_K: // delete all characters to the right of the cursor
            set_key_name("ctrl + k");
            while (shell.cursor < shell.length) {
                remove_char_at_cursor();
            }
            goto FINISHED;
        case KEY_CTRL_U: // delete all characters to the left of the cursor
            set_key_name("ctrl + u");
            while (shell.cursor > 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            goto FINISHED;

        case KEY_HT:
            set_key_name("tab");
            goto FINISHED;
        case KEY_BS: // delete one character to the left of the cursor
            set_key_name("backspace");
            if (shell.cursor != 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            goto FINISHED;
        case KEY_CR:
            set_key_name("enter");
            if (!shell.sequenceInspectionMode) {
                shell.buffer[shell.length] = '\0';
                println("");
                if (shell.length > 0) {
                    process_shell_command();

                    reset_current_line();
                }
                print(SHELL_PROMPT_STRING);
            }
            goto FINISHED;

        case KEY_CTRL_CR:
            set_key_name("ctrl + enter");
            goto FINISHED;
        default:
            return;
        }
    case 2:
        switch (currentChar) {
        case KEY_ALT_BS: // delete all characters to the left of the cursor
            set_key_name("ctrl + backspace");
            while (shell.cursor > 0) {
                move_cursor(-1);
                remove_char_at_cursor();
            }
            goto FINISHED;
        default:
            if (isalnum(currentChar)) {
                char name[] = "alt + x";
                name[6] = currentChar;
                set_key_name(&name);
                goto FINISHED;
            }
            return;
        }
    case 3:
        switch (currentChar) {
        case KEY_UP:
            set_key_name("up");
            goto FINISHED;
        case KEY_DOWN:
            set_key_name("down");
            goto FINISHED;
        case KEY_RIGHT:
            set_key_name("right");
            move_cursor(1);
            goto FINISHED;
        case KEY_LEFT:
            set_key_name("left");
            move_cursor(-1);
            goto FINISHED;
        case KEY_HOME:
            set_key_name("home");
            move_cursor_to(0);
            goto FINISHED;
        case KEY_END:
            set_key_name("end");
            move_cursor_to(shell.length);
            goto FINISHED;
        case KEY_F1:
            set_key_name("F1");
            goto FINISHED;
        case KEY_F2:
            set_key_name("F2");
            goto FINISHED;
        case KEY_F3:
            set_key_name("F3");
            goto FINISHED;
        case KEY_F4:
            set_key_name("F4");
            goto FINISHED;
        default:
            return;
        }
    case 4:
        switch (currentChar) {
        case '~':
            switch (prevChar) {
            case KEY_PGUP:
                set_key_name("pgup");
                goto FINISHED;
            case KEY_PGDN:
                set_key_name("pgdn");
                goto FINISHED;
            case KEY_DEL:
                set_key_name("delete");
                if (shell.cursor != shell.length) {
                    remove_char_at_cursor();
                }
                goto FINISHED;
            case KEY_INS:
                set_key_name("insert");
                goto FINISHED;
            }
        default:
            return;
        }
    case 5:
        if (currentChar == '~') {
            switch (prevChar) {
            case KEY_F5:
                set_key_name("F5");
                goto FINISHED;
            case KEY_F6:
                set_key_name("F6");
                goto FINISHED;
            case KEY_F7:
                set_key_name("F7");
                goto FINISHED;
            case KEY_F8:
                set_key_name("F8");
                goto FINISHED;
            case KEY_F9:
                toggle_sequence_inspection_mode();
                set_key_name("F9");
                goto FINISHED;
            case KEY_F10:
                set_key_name("F10");
                goto FINISHED;
            case KEY_F11:
                set_key_name("F11");
                goto FINISHED;
            case KEY_F12:
                set_key_name("F12");
                goto FINISHED;
            }
        default:
            return;
        }
    case 6:
        switch (currentChar) {
        case KEY_UP:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + up");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + up");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + up");
                goto FINISHED;
            }
        case KEY_DOWN:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + down");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + down");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + down");
                goto FINISHED;
            }
        case KEY_RIGHT:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + right");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + right");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + right");
                do {
                    move_cursor(1);
                } while (shell.buffer[shell.cursor] != ' ' &&
                         shell.cursor < shell.length);
                goto FINISHED;
            }
        case KEY_LEFT:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + left");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + left");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + left");
                do {
                    move_cursor(-1);
                } while (shell.buffer[shell.cursor] != ' ' && shell.cursor > 0);
                goto FINISHED;
            }
        case KEY_HOME:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + home");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + home");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + home");
                goto FINISHED;
            }
        case KEY_END:
            switch (sequence.buffer[5]) {
            case MOD_SHIFT:
                set_key_name("shift + end");
                goto FINISHED;
            case MOD_ALT:
                set_key_name("alt + end");
                goto FINISHED;
            case MOD_CTRL:
                set_key_name("ctrl + end");
                goto FINISHED;
            }
        case '~':
            switch (sequence.buffer[3]) {
            case KEY_DEL:
                switch (sequence.buffer[5]) {
                case MOD_SHIFT:
                    set_key_name("shift + delete");
                    goto FINISHED;
                case MOD_ALT:
                    set_key_name("alt + delete");
                    goto FINISHED;
                case MOD_CTRL:
                    set_key_name("ctrl + delete");
                    goto FINISHED;
                }
            case KEY_INS:
                switch (sequence.buffer[5]) {
                case MOD_SHIFT:
                    set_key_name("shift + insert");
                    goto FINISHED;
                case MOD_ALT:
                    set_key_name("alt + insert");
                    goto FINISHED;
                case MOD_CTRL:
                    set_key_name("ctrl + insert");
                    goto FINISHED;
                }
            }
        default:
            return;
        }
    case 7:
        goto FINISHED;
    }

FINISHED:
    // if we're in raw echo mode, then add a newline to seperate groups of
    // escape sequence codes
    if (shell.sequenceInspectionMode) {
        print_key_name();
        printf(" length: %d\r\n", sequence.length);
        println("");
    }

    // If we reach this spot, then we've fully processed the current
    // escape sequence, and we can exit escape mode.
    reset_sequence_buffer();
    shell.escapeMode = 0;
}