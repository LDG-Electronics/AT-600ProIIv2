#ifndef _SHELL_JSON_H_
#define _SHELL_JSON_H_

/* ************************************************************************** */

typedef enum {
    jsonString,
    jsonNumber,
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

extern void json_serialize_and_print(const json_field_t *object);

#endif