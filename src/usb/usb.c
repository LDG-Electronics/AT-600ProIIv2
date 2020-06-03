#include "usb.h"
#include "events.h"
#include "libraries/str_len.h"
#include "os/json/jsmn.h"
#include "os/json/json_print.h"
#include "os/log_macros.h"
#include "os/serial_port.h"
#include "os/stopwatch.h"
#include "os/system_time.h"
#include "peripherals/uart.h"
#include "usb.h"
#include "usb_json_hash.h"
#include "usb_messages.h"
#include "usb_port.h"
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
static uint8_t LOG_LEVEL = L_TRACE;

/* ************************************************************************** */

// Incoming JSON objects MUST BE shorter than this length
#define JSON_BUFFER_SIZE 256

typedef struct {
    char data[JSON_BUFFER_SIZE]; // The incoming JSON text goes here
    uint8_t length;              // The number of recieved bytes
    uint8_t depth;               // The current {} nesting level
    system_time_t messageStartTime;
    system_time_t lastCharacterTime;
    union {
        struct {
            unsigned timedout : 1;
            unsigned stalled : 1;
        };
        uint8_t errors;
    };
} json_buffer_t;

// These have to be file scope so they can be accessed by shell commands
static json_buffer_t buffer[2];
static uint8_t active = 0;

void reset_json_buffer(json_buffer_t *buffer) {
    memset(buffer, 0, sizeof(json_buffer_t));
}

void swap_active_buffer(void) {
    // swap which buffer is active
    if (active == 0) {
        active = 1;
    } else {
        active = 0;
    }

    // wipe the new buffer before using it
    reset_json_buffer(&buffer[active]);
}

/* ************************************************************************** */

void usb_init(uart_interface_t interface) {
    // pass the uart down to the usb port driver
    usb_port_init(interface);

    reset_json_buffer(&buffer[0]);
    reset_json_buffer(&buffer[1]);

    log_register();
}

/* ************************************************************************** */

void message_timeout_error(void) {
    LOG_INFO({ println("message_timeout_error"); });
    // It's been too long since the current message started.
    // We should scan what we've received for a message ID, then send back
    // an error message, then reset our buffer and try again
}

void message_stall_error(void) {
    LOG_INFO({ println("message_stall_error"); });
    // It's been too long since the last character was rx'd
    // We should scan what we've received for a message ID, then send back
    // an error message, then reset our buffer and try again
}

/* -------------------------------------------------------------------------- */

#define MESSAGE_MAXIMUM_TIME 100
#define MESSAGE_TIMEOUT_WINDOW 100

void insert_character(json_buffer_t *buf, char currentChar) {
    if ((currentChar == '{') && (buf->depth == 0)) {
        // LOG_INFO({ println("Start of new JSON message"); });
        buf->messageStartTime = get_current_time();
        buf->lastCharacterTime = get_current_time();
    }

    if (time_since(buf->messageStartTime) > MESSAGE_MAXIMUM_TIME) {
        buf->timedout = true;
    }
    if (time_since(buf->lastCharacterTime) > MESSAGE_TIMEOUT_WINDOW) {
        buf->stalled = true;
    }

    if (currentChar == '{') {
        buf->depth++;
    }

    if (buf->depth > 0) {
        buf->data[buf->length++] = currentChar;
        buf->lastCharacterTime = get_current_time();
    }

    if (currentChar == '}') {
        buf->depth--;
    }
}

/* -------------------------------------------------------------------------- */

// JSMN needs these for parsing
#define MAX_TOKENS 128
jsmntok_t tokens[MAX_TOKENS];
int tokensParsed;

#define TOKEN(number) &buf->data[tokens[number].start]

void preprocess(json_buffer_t *buf) {
    jsmn_parser parser;
    jsmn_init(&parser);

    // tokenize the buffer
    tokensParsed =
        jsmn_parse(&parser, buf->data, str_len(buf->data), tokens, MAX_TOKENS);

    // terminate each token in the original string
    for (uint8_t i = 0; i < tokensParsed; i++) {
        buf->data[tokens[i].end] = 0;
    }

    // hash the tokens
    for (uint8_t i = 0; i < tokensParsed; i++) {
        if (tokens[i].type == JSMN_STRING) {
            tokens[i].hash = json_hash(TOKEN(i));
        } else {
            tokens[i].hash = -1;
        }
    }
}

void respond(json_buffer_t *buf) {
    for (uint8_t i = 0; i < tokensParsed; i++) {
        switch (tokens[i].hash) {
            case hash_set_antenna:
                // select_antenna(atoi(TOKEN(i + 1)));
                json_print(usb_print, responseOk);
                break;

            case hash_message_id:
                LOG_DEBUG({
                    printf("Found a message_id at position [%d]: ", i);
                    println(TOKEN(i + 1));
                });
                set_json_messageID(TOKEN(i + 1));
                break;

            case hash_ping:
                set_json_pongString(TOKEN(i + 1));
                json_print(usb_print, responsePong);
                break;

            case hash_relays:
                switch (tokens[i + 1].hash) {
                    case hash_cup:
                        // relays_cup();
                        json_print(usb_print, responseOk);
                        break;
                    case hash_cdn:
                        // relays_cdn();
                        json_print(usb_print, responseOk);
                        break;
                    case hash_lup:
                        // relays_lup();
                        json_print(usb_print, responseOk);
                        break;
                    case hash_ldn:
                        // relays_ldn();
                        json_print(usb_print, responseOk);
                        break;
                }
                break;

            case hash_request_device_info:
                switch (tokens[i + 1].hash) {
                    case hash_short:
                        json_print(usb_print, deviceInfoShort);
                        break;
                    case hash_full:
                        json_print(usb_print, deviceInfoFull);
                        break;
                }
                break;
        }
    }
}

/* ************************************************************************** */

void usb_update(void) {
    char currentChar = usb_getch();

    // return early if we don't have a valid character
    if (!isprint(currentChar)) {
        return;
    }

    insert_character(&buffer[active], currentChar);

    // These conditions mean we've reached the end of a JSON object
    if ((buffer[active].length > 0) && (buffer[active].depth == 0)) {
        // add terminating null, after closing brace
        buffer[active].data[buffer[active].length] = 0;

        LOG_INFO({
            print("Complete JSON message recieved in ");
            printf("%lu mS\r\n", time_since(buffer[active].messageStartTime));
        });
        LOG_DEBUG({
            print("JSON message: [");
            print(&buffer[active].data);
            println("]");
        });

        preprocess(&buffer[active]);
        LOG_INFO({
            print("Preprocessing completed in: ");
            printf("%lu mS\r\n", time_since(buffer[active].messageStartTime));
        });

        respond(&buffer[active]);
        LOG_INFO({
            print("Response completed in: ");
            printf("%lu mS\r\n", time_since(buffer[active].messageStartTime));
        });

        swap_active_buffer();
    }
}
