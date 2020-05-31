#ifndef _USB_JSON_HASH_H_
#define _USB_JSON_HASH_H_

#include <stdint.h>

/* ************************************************************************** */

typedef enum {
    hash_ok = 0,
    hash_ping = 1,
    hash_command = 2,
    hash_patch = 3,
    hash_compile_date = 4,
    hash_message_id = 5,
    hash_pong = 6,
    hash_short = 7,
    hash_toggle = 8,
    hash_software_version = 9,
    hash_device_info = 10,
    hash_major = 11,
    hash_auto = 12,
    hash_compiler_version = 13,
    hash_full = 14,
    hash_minor = 15,
    hash_name = 16,
    hash_set_antenna = 17,
    hash_request_device_info = 18,
    hash_serial = 19,
    hash_compile_time = 20,
    hash_response = 21,
} hash_value_t;

extern hash_value_t json_hash(const char *string);

#endif /* _USB_JSON_HASH_H_ */