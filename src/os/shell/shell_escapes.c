#include "../../includes.h"
#include "shell_internals.h"

/* ************************************************************************** */

#define ESCAPE_BUFFER_LENGTH 10

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
    case 2:
        // fallthrough
        return;
    case 3:
        switch (currentChar) {
        case KEY_UP:
            // print("up");
            goto FINISHED;
        case KEY_DOWN:
            // print("down");
            goto FINISHED;
        case KEY_RIGHT:
            // print("right");
            move_cursor(1);
            goto FINISHED;
        case KEY_LEFT:
            // print("left");
            move_cursor(-1);
            goto FINISHED;
        case KEY_HOME:
            // print("home");
            move_cursor_to(0);
            goto FINISHED;
        case KEY_END:
            // print("end");
            move_cursor_to(shell.length);
            goto FINISHED;
        case KEY_F1:
            // print("F1");
            goto FINISHED;
        case KEY_F2:
            // print("F2");
            goto FINISHED;
        case KEY_F3:
            // print("F3");
            goto FINISHED;
        case KEY_F4:
            // print("F4");
            goto FINISHED;
        }
        return;
    case 4:
        switch (currentChar) {
        case '~':
            switch (prevChar) {
            case KEY_PGUP:
                // print("pgup");
                goto FINISHED;
            case KEY_PGDN:
                // print("pgdn");
                goto FINISHED;
            case KEY_DEL:
                // println("del");
                if (shell.cursor != shell.length) {
                    remove_char_at_cursor();
                }
                goto FINISHED;
            case KEY_INS:
                // print("ins");
                goto FINISHED;
            }
        }
        return;
    case 5:
        if (currentChar == '~') {
            switch (prevChar) {
            case KEY_F5:
                // print("F5");
                goto FINISHED;
            case KEY_F6:
                // print("F6");
                goto FINISHED;
            case KEY_F7:
                // print("F7");
                goto FINISHED;
            case KEY_F8:
                // print("F8");
                goto FINISHED;
            case KEY_F9:
                // print("F9");
                toggle_raw_echo_mode();
                goto FINISHED;
            case KEY_F10:
                // print("F10");
                goto FINISHED;
            case KEY_F11:
                // print("F11");
                goto FINISHED;
            case KEY_F12:
                // print("F12");
                goto FINISHED;
            }
        }
        return;
    case 6:
        switch (currentChar) {
        case KEY_RIGHT:
            // print("^right");
            goto FINISHED;
        case KEY_LEFT:
            // print("^left");
            goto FINISHED;
        case KEY_HOME:
            // print("^home");
            goto FINISHED;
        case KEY_END:
            // print("^end");
            goto FINISHED;
        case '~':
            switch (escape.buffer[3]) {
            case KEY_DEL:
                // print("^delete");
                goto FINISHED;
            case KEY_END:
                // print("^insert");
                goto FINISHED;
            }
        }
        return;
    case 7:
        goto FINISHED;

    FINISHED:
        // if we're in raw echo mode, then add a newline to seperate groups
        // of escape sequence codes
        if (shell.rawEchoMode) {
            printf(" length: %d\r\n", escape.length);
            println("");
        }

        // If we reach this spot, then we've fully processed the current
        // escape sequence, and we can exit escape mode.
        reset_escape_buffer();
        shell.escapeMode = 0;
    }
}