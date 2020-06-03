#ifndef _USB_JSON_HASH_H_
#define _USB_JSON_HASH_H_

#include <stdint.h>

/* ************************************************************************** */

typedef enum {
    hash_message_id = 0,
    hash_serial = 1,
    hash_major = 2,
    hash_minor = 3,
    hash_patch = 4,
    hash_compiler_version = 5,
    hash_compile_date = 6,
    hash_compile_time = 7,
    hash_request_device_info = 8,
    hash_ping = 9,
    hash_command = 10,
    hash_set_antenna = 11,
    hash_auto = 12,
    hash_toggle = 13,
    hash_short = 14,
    hash_full = 15,
    hash_relays = 16,
    hash_cup = 17,
    hash_cdn = 18,
    hash_lup = 19,
    hash_ldn = 20,
    hash_device_info = 21,
    hash_name = 22,
    hash_software_version = 23,
    hash_ok = 24,
    hash_response = 25,
    hash_pong = 26,
} hash_value_t;

extern hash_value_t json_hash(const char *string);

#endif /* _USB_JSON_HASH_H_ */