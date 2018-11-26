#include "shell_json.h"
#include "../serial_port.h"
#include <stdint.h>

/* ************************************************************************** */

// prints a single key:value pair
void print_json_field(const json_field_t *field) {
    // print the key
    printf("\"%s\":", field->string);

    // print the value
    switch (field->type) {
    case jsonU16:
        uint16_t tempU16 = *(uint16_t *)field->value;
        printf("%u", tempU16);
        return;
    case jsonFloat:
        float tempFloat = *(float *)field->value;
        printf("%f", tempFloat);
        return;
    default:
        // type not supported
        print("NULL");
        return;
    }
}

// loops through a provided json object until it hits null-termination
void json_serialize_and_print(const json_field_t *object) {
    print("{");

    while (object->string != NULL) {
        print_json_field(object++);
        if (object->string != NULL) {
            print(",");
        }
    }

    println("}");
}