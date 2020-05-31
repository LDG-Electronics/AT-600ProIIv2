#include "usb_json_hash.h"
#include <stdint.h>
#include <string.h>

/* ************************************************************************** */
/* [[[cog
    import codegen as code
    from collections import OrderedDict 

    keys = utils.search('src/usb/usb_messages.c', r'(?<={nKey, ").*?(?="})')
    keys = list(OrderedDict.fromkeys(keys))  
    prefix = 'hash_'
    enum = code.Enum(
        name = 'hash_value_t',
        values = keys,
        prefix = prefix,
        typedef = True,
        explicit = True
    )

    struct = code.Struct(
        'key',
        ['const char *name', f'{enum.name} value'],
    ).assemble()

    cog.outl()
    cog.outl(utils.Gperf([f'{k}, {prefix}{k}' for k in keys], struct).run())

    func = code.Function(
        name = 'json_hash',
        return_type = enum.name,
        params = 'const char *string',
        extern = True
    )

    code.Header(
        name = cog.inFile.replace('.c', '.h'),
        includes = '<stdint.h>',
        contents = [enum.assemble(), func.declaration()]
    ).write()

]]] */

/* ANSI-C code produced by gperf version 3.1 */
/* Command-line: 'C:\\ProgramData\\chocolatey\\lib\\gperf\\tools\\gperf.exe' -m
 * 100 -r -n strings.gperf  */
/* Computed positions: -k'1-2,9' */

#if !(                                                                         \
    (' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) && ('%' == 37) && \
    ('&' == 38) && ('\'' == 39) && ('(' == 40) && (')' == 41) &&               \
    ('*' == 42) && ('+' == 43) && (',' == 44) && ('-' == 45) && ('.' == 46) && \
    ('/' == 47) && ('0' == 48) && ('1' == 49) && ('2' == 50) && ('3' == 51) && \
    ('4' == 52) && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) && \
    ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) && ('=' == 61) && \
    ('>' == 62) && ('?' == 63) && ('A' == 65) && ('B' == 66) && ('C' == 67) && \
    ('D' == 68) && ('E' == 69) && ('F' == 70) && ('G' == 71) && ('H' == 72) && \
    ('I' == 73) && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) && \
    ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) && ('R' == 82) && \
    ('S' == 83) && ('T' == 84) && ('U' == 85) && ('V' == 86) && ('W' == 87) && \
    ('X' == 88) && ('Y' == 89) && ('Z' == 90) && ('[' == 91) &&                \
    ('\\' == 92) && (']' == 93) && ('^' == 94) && ('_' == 95) &&               \
    ('a' == 97) && ('b' == 98) && ('c' == 99) && ('d' == 100) &&               \
    ('e' == 101) && ('f' == 102) && ('g' == 103) && ('h' == 104) &&            \
    ('i' == 105) && ('j' == 106) && ('k' == 107) && ('l' == 108) &&            \
    ('m' == 109) && ('n' == 110) && ('o' == 111) && ('p' == 112) &&            \
    ('q' == 113) && ('r' == 114) && ('s' == 115) && ('t' == 116) &&            \
    ('u' == 117) && ('v' == 118) && ('w' == 119) && ('x' == 120) &&            \
    ('y' == 121) && ('z' == 122) && ('{' == 123) && ('|' == 124) &&            \
    ('}' == 125) && ('~' == 126))
    /* The character set is not based on ISO-646.  */
    #error                                                                     \
        "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gperf@gnu.org>."
#endif

struct key {
    const char *name;
    hash_value_t value;
};

#define TOTAL_KEYWORDS 22
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 19
#define MIN_HASH_VALUE 0
#define MAX_HASH_VALUE 21
/* maximum key range = 22, duplicates = 0 */

#ifdef __GNUC__

#else
    #ifdef __cplusplus

    #endif
#endif
static uint16_t hash(const char *str, uint8_t len) {
    static const uint8_t asso_values[] = {
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 7,  22, 2,  22, 1,  3,  3,  5,  22, 18, 0,  22, 12,
        22, 1,  4,  7,  0,  22, 11, 2,  5,  16, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
        22, 22, 22, 22};
    uint16_t hval = 0;

    switch (len) {
        default:
            hval += asso_values[(uint8_t)str[8]];
        /*FALLTHROUGH*/
        case 8:
        case 7:
        case 6:
        case 5:
        case 4:
        case 3:
        case 2:
            hval += asso_values[(uint8_t)str[1]];
        /*FALLTHROUGH*/
        case 1:
            hval += asso_values[(uint8_t)str[0]];
            break;
    }
    return hval;
}

static const struct key wordlist[] = {
    {"ping", hash_ping},
    {"minor", hash_minor},
    {"patch", hash_patch},
    {"major", hash_major},
    {"message_id", hash_message_id},
    {"serial", hash_serial},
    {"name", hash_name},
    {"pong", hash_pong},
    {"command", hash_command},
    {"set_antenna", hash_set_antenna},
    {"device_info", hash_device_info},
    {"compile_date", hash_compile_date},
    {"toggle", hash_toggle},
    {"compile_time", hash_compile_time},
    {"response", hash_response},
    {"compiler_version", hash_compiler_version},
    {"software_version", hash_software_version},
    {"request_device_info", hash_request_device_info},
    {"auto", hash_auto},
    {"ok", hash_ok},
    {"short", hash_short},
    {"full", hash_full},
};

const struct key *in_word_set(const char *str, uint8_t len) {
    if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH) {
        uint16_t key = hash(str, len);

        if (key <= MAX_HASH_VALUE) {
            const char *s = wordlist[key].name;

            if (*str == *s && !strcmp(str + 1, s + 1))
                return &wordlist[key];
        }
    }
    return 0;
}

/* [[[end]]] */
/* ************************************************************************** */

#if TOTAL_KEYWORDS != (MAX_HASH_VALUE + 1)
    #error Gperf output is not minimal! Add more iterations!
#endif

hash_value_t json_hash(const char *string) {
    const struct key *result = in_word_set(string, strlen(string));

    return result->value;
}

/* ************************************************************************** */

#include "os/shell/shell_command_utils.h"

void sh_hash(int argc, char **argv) {
    if (argc == 1) {
        println("you didn't give me a string");
        return;
    }

    for (uint8_t i = 1; i < argc; i++) {
        const struct key *result = in_word_set(argv[i], strlen(argv[i]));

        if (!result) {
            printf("'%s' wasn't in the word list\r\n", argv[i]);
        } else {
            printf("found '%s' [%d]\r\n", result->name, result->value);
        }
    }
}

REGISTER_SHELL_COMMAND(sh_hash, "hash");