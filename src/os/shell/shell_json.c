#include "shell_json.h"
#include "../console_io.h"

/* ************************************************************************** */

// prints a single key:value pair
void print_json_field(const json_field_t *field) {
    switch (field->type) {
    case jsonNumber:
        printf("\"%s\":%f", field->string, field->value);
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