#include "shell_keys.h"
#include "../log_macros.h"
static uint8_t LOG_LEVEL = L_SILENT;

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

typedef enum {
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
    LOG_TRACE({ println("decode_escape_sequence"); });

    key_t newKey = {UNKNOWN, NONE};

    switch (sequence.length) {
    case 0:
        newKey.key = ESCAPE;
        return newKey;
    case 2:
        switch (sequence.buffer[1]) {
        default:
            return newKey;
        case KEY_UP:
            newKey.key = UP;
            return newKey;
        case KEY_DOWN:
            newKey.key = DOWN;
            return newKey;
        case KEY_RIGHT:
            newKey.key = RIGHT;
            return newKey;
        case KEY_LEFT:
            newKey.key = LEFT;
            return newKey;
        case KEY_HOME:
            newKey.key = HOME;
            return newKey;
        case KEY_END:
            newKey.key = END;
            return newKey;
        case KEY_F1:
            newKey.key = F1;
            return newKey;
        case KEY_F2:
            newKey.key = F2;
            return newKey;
        case KEY_F3:
            newKey.key = F3;
            return newKey;
        case KEY_F4:
            newKey.key = F4;
            return newKey;
        }
    case 3:
        if (sequence.buffer[2] == '~') {
            switch (sequence.buffer[1]) {
            default:
                return newKey;
            case KEY_HOME2:
                newKey.key = HOME;
                return newKey;
            case KEY_PGUP:
                newKey.key = PAGEUP;
                return newKey;
            case KEY_PGDN:
                newKey.key = PAGEDOWN;
                return newKey;
            case KEY_DEL:
                newKey.key = DELETE;
                return newKey;
            case KEY_INS:
                newKey.key = INSERT;
                return newKey;
            }
        }
        return newKey;
    case 4:
        if (sequence.buffer[3] == '~') {
            switch (sequence.buffer[2]) {
            default:
                return newKey;
            case KEY_F5:
            case KEY_F5_ALT:
                newKey.key = F5;
                return newKey;
            case KEY_F6:
                newKey.key = F6;
                return newKey;
            case KEY_F7:
                newKey.key = F7;
                return newKey;
            case KEY_F8:
                newKey.key = F8;
                return newKey;
            case KEY_F9:
                newKey.key = F9;
                return newKey;
            case KEY_F10:
                newKey.key = F10;
                return newKey;
            case KEY_F11:
                newKey.key = F11;
                return newKey;
            case KEY_F12:
                newKey.key = F12;
                return newKey;
            }
        }
        return newKey;
    case 5:
        switch (sequence.buffer[4]) {
        default:
            return newKey;
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
        }
        newKey.mod = sequence.buffer[3] - '0';
        return newKey;
    case 6:
        if (sequence.buffer[5] == '~') {
            switch (sequence.buffer[2]) {
            default:
                return newKey;
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
            }
            newKey.mod = sequence.buffer[4] - '0';
            return newKey;
        }
        return newKey;
    default:
        return newKey;
    }
}

typedef enum {
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
    LOG_TRACE({ println("decode_control_character"); });
    LOG_DEBUG({ printf("currentChar: %d\r\n", currentChar); });

    key_t newKey = {UNKNOWN, NONE};

    switch (currentChar) {
    default:
        return newKey;
    case KEY_BS:
        newKey.key = BACKSPACE;
        return newKey;
    case KEY_LF:
    case KEY_CR:
        newKey.key = ENTER;
        return newKey;
    case KEY_TAB:
        newKey.key = TAB;
        return newKey;
    }
}

void intercept_escape_sequence(void) {
    LOG_TRACE({ println("intercept_escape_sequence"); });
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

    LOG_DEBUG({ printf("sequence.buffer: '%s'\r\n", sequence.buffer); });
    LOG_DEBUG({ printf("sequence.length: %d\r\n", sequence.length); });
}

key_t identify_key(char currentChar) {
    LOG_TRACE({ println("identify_key"); });

    key_t key = {UNKNOWN, NONE};

    if (currentChar == KEY_ESC) {
        intercept_escape_sequence();
        key = decode_escape_sequence();
    } else {
        key = decode_control_character(currentChar);
    }

    LOG_DEBUG(print_key(&key););
    return key;
}