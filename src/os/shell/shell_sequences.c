#include "../../includes.h"
#include "shell_internals.h"
#define LOG_LEVEL logLevel
static uint8_t logLevel = L_SILENT;

/* ************************************************************************** */

#define X(NAME) #NAME,
const char *keyNameString[] = {KEY_NAME_LIST};
const char *keyModifierString[] = {KEY_MODIFIER_LIST};
#undef X

typedef struct {
    char buffer[SEQUENCE_BUFFER_LENGTH];
    uint8_t length;
} sequence_t;

sequence_t sequence;

void shell_sequences_init(void) {
    memset(&sequence, NULL, sizeof(sequence));
    log_register();
}

/* -------------------------------------------------------------------------- */

void print_key(key_t *key) {
    if (key->mod != NONE) {
        print(keyModifierString[key->mod]);
        print(" + ");
    }
    println(keyNameString[key->key]);
}

/* -------------------------------------------------------------------------- */

enum {
    KEY_UP = 65,
    KEY_DOWN = 66,
    KEY_RIGHT = 67,
    KEY_LEFT = 68,
    KEY_HOME = 72,
    KEY_HOME2 = 49,
    KEY_END = 70,
    KEY_DEL = 51,
    KEY_INS = 50,
    KEY_PGUP = 53,
    KEY_PGDN = 54,
    KEY_F1 = 80,
    KEY_F2 = 81,
    KEY_F3 = 82,
    KEY_F4 = 83,
    KEY_F5 = 54,
    KEY_F5_ALT = 53,
    KEY_F6 = 55,
    KEY_F7 = 56,
    KEY_F8 = 57,
    KEY_F9 = 48,
    KEY_F10 = 49,
    KEY_F11 = 51,
    KEY_F12 = 52,
} sequenceIdentifiers;

key_t decode_escape_sequence(void) {
    log_trace(println("decode_escape_sequence"););
    log_debug(printf("sequence.length: %d\r\n", sequence.length););

    key_t newKey = {UNKNOWN, NONE};

    switch (sequence.length) {
    case 0:
        newKey.key = ESCAPE;
        goto FINISHED;
    case 2:
        switch (sequence.buffer[1]) {
        case KEY_UP:
            newKey.key = UP;
            goto FINISHED;
        case KEY_DOWN:
            newKey.key = DOWN;
            goto FINISHED;
        case KEY_RIGHT:
            newKey.key = RIGHT;
            goto FINISHED;
        case KEY_LEFT:
            newKey.key = LEFT;
            goto FINISHED;
        case KEY_HOME:
            newKey.key = HOME;
            goto FINISHED;
        case KEY_END:
            newKey.key = END;
            goto FINISHED;
        case KEY_F1:
            newKey.key = F1;
            goto FINISHED;
        case KEY_F2:
            newKey.key = F2;
            goto FINISHED;
        case KEY_F3:
            newKey.key = F3;
            goto FINISHED;
        case KEY_F4:
            newKey.key = F4;
            goto FINISHED;
        default:
            goto FINISHED;
        }
    case 3:
        if (sequence.buffer[2] == '~') {
            switch (sequence.buffer[1]) {
            case KEY_HOME2:
                newKey.key = HOME;
                goto FINISHED;
            case KEY_PGUP:
                newKey.key = PAGEUP;
                goto FINISHED;
            case KEY_PGDN:
                newKey.key = PAGEDOWN;
                goto FINISHED;
            case KEY_DEL:
                newKey.key = DELETE;
                goto FINISHED;
            case KEY_INS:
                newKey.key = INSERT;
                goto FINISHED;
            }
        }
        goto FINISHED;
    case 4:
        if (sequence.buffer[3] == '~') {
            switch (sequence.buffer[2]) {
            case KEY_F5:
            case KEY_F5_ALT:
                newKey.key = F5;
                goto FINISHED;
            case KEY_F6:
                newKey.key = F6;
                goto FINISHED;
            case KEY_F7:
                newKey.key = F7;
                goto FINISHED;
            case KEY_F8:
                newKey.key = F8;
                goto FINISHED;
            case KEY_F9:
                newKey.key = F9;
                goto FINISHED;
            case KEY_F10:
                newKey.key = F10;
                goto FINISHED;
            case KEY_F11:
                newKey.key = F11;
                goto FINISHED;
            case KEY_F12:
                newKey.key = F12;
                goto FINISHED;
            default:
                goto FINISHED;
            }
        }
        goto FINISHED;
    case 5:
        switch (sequence.buffer[4]) {
        case KEY_F1:
            newKey.key = F1;
            break;
        case KEY_F2:
            newKey.key = F2;
            break;
        case KEY_F3:
            newKey.key = F3;
            break;
        case KEY_F4:
            newKey.key = F4;
            break;
        case KEY_UP:
            newKey.key = UP;
            break;
        case KEY_DOWN:
            newKey.key = DOWN;
            break;
        case KEY_RIGHT:
            newKey.key = RIGHT;
            break;
        case KEY_LEFT:
            newKey.key = LEFT;
            break;
        case KEY_HOME:
            newKey.key = HOME;
            break;
        case KEY_END:
            newKey.key = END;
            break;
        case '~':
            if (sequence.buffer[1] == KEY_DEL) {
                newKey.key = DELETE;
                break;
            }
            if (sequence.buffer[1] == KEY_INS) {
                newKey.key = INSERT;
                break;
            }
        default:
            goto FINISHED;
        }
        newKey.mod = sequence.buffer[3] - '0';
        goto FINISHED;
    case 6:
        if (sequence.buffer[5] == '~') {
            switch (sequence.buffer[2]) {
            case KEY_F5:
            case KEY_F5_ALT:
                newKey.key = F5;
                break;
            case KEY_F6:
                newKey.key = F6;
                break;
            case KEY_F7:
                newKey.key = F7;
                break;
            case KEY_F8:
                newKey.key = F8;
                break;
            case KEY_F9:
                newKey.key = F9;
                break;
            case KEY_F10:
                newKey.key = F10;
                break;
            case KEY_F11:
                newKey.key = F11;
                break;
            case KEY_F12:
                newKey.key = F12;
                break;
            default:
                goto FINISHED;
            }
            newKey.mod = sequence.buffer[4] - '0';
            goto FINISHED;
        }
        goto FINISHED;
    default:
        goto FINISHED;
    }

FINISHED:
    log_debug(print_key(&newKey););
    return newKey;
}

enum {
    KEY_ETX = 3,
    KEY_CTRL_D = 4,
    KEY_CTRL_E = 5,
    KEY_BS = 8,
    KEY_TAB = 9,
    KEY_LF = 10,
    KEY_CR = 13,
    KEY_CTRL_U = 21,
    KEY_CTRL_Y = 25,
    KEY_ESC = 27,
    KEY_CTRL_Z = 26,
    KEY_CTRL_BS = 31,
} controlCharacters;

key_t decode_control_character(char currentChar) {
    log_trace(println("decode_control_character"););
    log_debug(printf("currentChar: %d\r\n", currentChar););

    key_t newKey = {UNKNOWN, NONE};

    switch (currentChar) {
    case KEY_BS:
        newKey.key = BACKSPACE;
        break;
    case KEY_LF:
    case KEY_CR:
        newKey.key = ENTER;
        break;
    case KEY_TAB:
        newKey.key = TAB;
        break;
    default:
        break;
    }

    log_debug(print_key(&newKey););
    return newKey;
}

void intercept_escape_sequence(void) {
    memset(&sequence, NULL, sizeof(sequence));

    system_time_t startTime = systick_read();
    while (systick_elapsed_time(startTime) < 2) {
        // check for a new character
        sequence.buffer[sequence.length] = getch();
        // if valid character, move to next spot in buffer
        if (sequence.buffer[sequence.length] != 0) {
            sequence.length++;
        }
    }
}

key_t identify_key(char currentChar) {
    log_trace(println("intercept_escape_sequence"););

    key_t key = {UNKNOWN, NONE};

    if (currentChar == KEY_ESC) {
        intercept_escape_sequence();
        log_debug(printf("'%s'\r\n", sequence.buffer););
        key = decode_escape_sequence();
    } else {
        key = decode_control_character(currentChar);
    }

    return key;
}