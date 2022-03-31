#ifndef _MESSAGES_H_
#define _MESSAGES_H_

#include "os/judi/judi.h"
#include "os/judi/judi_messages.h"
#include "os/judi/message_builder.h"
#include "os/usb_port.h"

/* ************************************************************************** */
// project specific message components

extern const json_node_t relaysObject[];
extern const json_node_t relaysCaps[];
extern const json_node_t relaysInput[];
extern const json_node_t relaysOutput[];
extern const json_node_t relaysAll[];
extern const json_node_t updateCurrentRelays[];

/* ************************************************************************** */

extern void respond(json_buffer_t *buf);

#endif // _MESSAGES_H_