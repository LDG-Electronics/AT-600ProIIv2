#ifndef _USB_MESSAGES_H_
#define _USB_MESSAGES_H_

#include "os/json/json_node.h"
#include <stdint.h>

/* ************************************************************************** */
/*  Data fields that are referenced by JSON nodes

*/

#define MESSAGE_ID_MAX_SIZE 32

extern void set_json_messageID(char *string);

#define PONG_STRING_MAX_SIZE 32

extern void set_json_pongString(char *string);

/* -------------------------------------------------------------------------- */
/*  Complete JSON message definitions

*/
extern const json_node_t msgDeviceInfo[];
extern const json_node_t deviceInfoShort[];
extern const json_node_t deviceInfoFull[];

extern const json_node_t msgOk[];

extern const json_node_t responseOk[];
extern const json_node_t responseError[];
extern const json_node_t responsePong[];

/* ************************************************************************** */

#endif // _USB_MESSAGES_H_