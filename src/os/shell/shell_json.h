#ifndef _SHELL_JSON_H_
#define _SHELL_JSON_H_

/* ************************************************************************** */

typedef enum {
    jsonString,
    jsonDouble,
    jsonU8,
    jsonI8,
    jsonU16,
    jsonI16,
    jsonU24,
    jsonI24,
    jsonU32,
    jsonI32,
    jsonObject,
    jsonArray,
    jsonTrue,
    jsonFalse,
    jsonNull,
    jsonBool,
} json_type_t;

typedef struct {
    const char *string;
    void *value;
    json_type_t type;
} json_field_t;

/* ************************************************************************** */

extern void print_json_field(const json_field_t *field);
extern void json_serialize_and_print(const json_field_t *object);

#endif