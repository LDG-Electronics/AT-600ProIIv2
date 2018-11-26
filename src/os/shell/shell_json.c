#include "shell_json.h"
#include "../serial_port.h"
#include <stdint.h>

/* ************************************************************************** */

// prints a single key:value pair
void print_json_field(const json_field_t *field) {
    float tempFloat;
    uint32_t tempUnsigned;

    switch (field->type) {
    case jsonU16:
        tempUnsigned = *(uint16_t *)field->value;
        printf("\"%s\":%lu", field->string, tempUnsigned);
        return;
    case jsonFloat:
        tempFloat = *(float *)field->value;
        printf("\"%s\":%f", field->string, tempFloat);
        return;
    default:
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