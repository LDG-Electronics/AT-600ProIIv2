#include "messages.h"
#include "display.h"
#include "events.h"
#include "flags.h"
#include "hash.h"
#include "os/json/json_print.h"
#include "os/judi/hash.h"
#include "os/judi/message_id.h"
#include "os/serial_port.h"
#include "relays.h"
#include "rf_sensor.h"
#include "system.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* ************************************************************************** */

bool update_allowed = false;

const json_node_t rfUpdate[] = {
    {nControl, "{"},   //
    {MESSAGE_ID_NODE}, //
    {nKey, "update"},  //
    {nControl, "{"},   //
    {nKey, "rf"},      //
    {nControl, "{"},   //

    {nKey, "fwdV"},    {nFloat, &currentRF.forwardVolts}, //
    {nKey, "revV"},    {nFloat, &currentRF.reverseVolts}, //
    {nKey, "matchQ"},  {nFloat, &currentRF.matchQuality}, //
    {nKey, "fwd"},     {nFloat, &currentRF.forwardWatts}, //
    {nKey, "rev"},     {nFloat, &currentRF.reverseWatts}, //
    {nKey, "swr"},     {nFloat, &currentRF.swr},          //

    {nKey, "freq"},    {nU16, &currentRF.frequency},

    {nControl, "\e"},
};

const json_node_t relaysUpdate[] = {
    {nKey, "relays"},              //
    {nControl, "{"},               //
    {nKey, "caps"},                //
    {nU8, &currentRelays[1].caps}, //
    {nKey, "inds"},                //
    {nU8, &currentRelays[1].inds}, //
    {nKey, "z"},                   //
    {nU8, &currentRelays[1].z},    //
    {nControl, "\e"},              //
};

void send_relay_update(void) {
    add_nodes(updatePreamble);
    add_nodes(relaysUpdate);
    print_message(usb_print);
}

/* ************************************************************************** */

#define HASH(number) buf->tokens[number].hash

void print_message_structure(json_buffer_t *buf) {
    usb_print("{\"debug\":\"");
    for (uint8_t i = 0; i < buf->tokensParsed; i++) {
        switch (buf->tokens[i].type) {
        case jsmn_undefined:
            usb_print("{");
            usb_print("undefined");
            usb_print("}");
            break;
        case jsmn_object:
            usb_print("{");
            usb_print("object");
            usb_print("}");
            break;
        case jsmn_array:
            usb_print("{");
            usb_print("array");
            usb_print("}");
            break;
        case jsmn_string:
            usb_print("{");
            usb_print(TOKEN(i));
            usb_print("}");
            break;
        case jsmn_primitive:
            usb_print("{");
            usb_print("primitive");
            usb_print("}");
            break;
        }
    }
    usb_print("\"}");
}

void respond(json_buffer_t *buf) {
    // print_message_structure(buf);

    // handle requests
    uint8_t request = find_key(buf, ROOT_OBJECT, hash_request);
    if (request) {
        switch (HASH(request + 1)) {
        case hash_device_info:
            update_allowed = false;
            add_nodes(updatePreamble);
            add_nodes(deviceInfo);
            print_message(usb_print);
            break;
        case hash_enable_updates:
            update_allowed = true;
            json_print(usb_print, responseOk);
            break;
        case hash_disable_updates:
            update_allowed = false;
            json_print(usb_print, responseOk);
            break;
        case hash_rf:
            json_print(usb_print, rfUpdate);
            break;
        case hash_relays:
            send_relay_update();
            break;
        }
    }

    // handle commands
    relays_t relays = read_current_relays();
    uint8_t caps, inds, z, relays_object;
    uint8_t command = find_key(buf, ROOT_OBJECT, hash_command);
    if (command) {
        switch (HASH(command + 1)) {
        case hash_locate:
            // locate_device();
            json_print(usb_print, responseOk);
            break;
        case hash_set_relays:
            relays_object = find_key(buf, ROOT_OBJECT, hash_relays);
            caps = find_key(buf, relays_object + 1, hash_caps);
            if (caps) {
                relays.caps = atoi(TOKEN(caps + 1));
            }
            inds = find_key(buf, relays_object + 1, hash_inds);
            if (inds) {
                relays.inds = atoi(TOKEN(inds + 1));
            }
            z = find_key(buf, relays_object + 1, hash_z);
            if (z) {
                relays.z = atoi(TOKEN(z + 1));
            }
            put_relays(relays);
            send_relay_update();
            break;
        case hash_cup:
            if (relays.caps < NUM_OF_CAPACITORS) {
                relays.caps++;
                put_relays(relays);
            }
            send_relay_update();
            break;
        case hash_cdn:
            if (relays.caps) {
                relays.caps--;
                put_relays(relays);
            }
            send_relay_update();
            break;
        case hash_lup:
            if (relays.inds < NUM_OF_INDUCTORS) {
                relays.inds++;
                put_relays(relays);
            }
            send_relay_update();
            break;
        case hash_ldn:
            if (relays.inds) {
                relays.inds--;
                put_relays(relays);
            }
            send_relay_update();
            break;
        }
    }
}