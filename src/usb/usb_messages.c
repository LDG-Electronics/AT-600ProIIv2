#include "usb_messages.h"
#include "hardware.h"
#include "os/json/json_print.h"
#include "peripherals/device_information.h"
#include <stdint.h>
#include <string.h>

/* ************************************************************************** */
/*  Constant data referenced by JSON nodes

    None of these pieces of data can change at runtime, but they still have to
    have memory allocated so they can be referenced by a void pointer in a JSON
    node.
*/
// product name
static const char productName[] = PRODUCT_NAME;

// product software version
static const uint16_t productVerMajor = PRODUCT_V_MAJOR;
static const uint16_t productVerMinor = PRODUCT_V_MINOR;
static const uint16_t productVerPatch = PRODUCT_V_PATCH;

// compilation information
static const uint16_t xc8Version = __XC8_VERSION;
static const char compileDate[] = __DATE__;
static const char compileTime[] = __TIME__;

/* -------------------------------------------------------------------------- */
/*  Non-constant data reference by JSON nodes

    These are pieces of data that aren't known at compile time, so they need to
    both be referenced by void pointer AND have a way to be updated from the
    program.
*/

// This is the message ID recieved from the host computer
static char json_messageID[MESSAGE_ID_MAX_SIZE] = "?";

void set_json_messageID(char *string) {
    //
    strncpy(json_messageID, string, MESSAGE_ID_MAX_SIZE);
}

/* ************************************************************************** */
/*  Reusable objects

*/

/*  The ID number of the message we're currently responding to
    "message_id" : <json_messageID>
*/
static const json_node_t message_ID[] = {
    {nKey, "message_id"},       //
    {nString, &json_messageID}, //
    {nControl, "\e"},           //
};

/*  This unit's unique serial number, programmed at the factory by Microchip
    "serial" : <hexMUI>
*/
static const json_node_t serial_number[] = {
    {nKey, "serial"},   //
    {nString, &hexMUI}, //
    {nControl, "\e"},   //
};

/*  The version of this product's software,
    "major" : <ver#>, "minor" : <ver#>, "patch" : <ver#>
*/
static const json_node_t software_version[] = {
    {nKey, "major"},          //
    {nU16, &productVerMajor}, //
    {nKey, "minor"},          //
    {nU16, &productVerMinor}, //
    {nKey, "patch"},          //
    {nU16, &productVerPatch}, //
    {nControl, "\e"},         //
};

/*  The version of the shell code

    "compiler_version" : <__XC8_VERSION>,
    "compile_date" : <__DATE__>,
    "compile_time" : <__TIME__>
*/
static const json_node_t compilation_info[] = {
    {nKey, "compiler_version"}, //
    {nU16, &xc8Version},        //
    {nKey, "compile_date"},     //
    {nString, &compileDate},    //
    {nKey, "compile_time"},     //
    {nString, &compileTime},    //
    {nControl, "\e"},           //
};

/* -------------------------------------------------------------------------- */
/*  Keys from messages that might be recieved, but will never be sent:

    {nKey, "request_device_info"}
    {nKey, "ping"}
    {nKey, "command"}
    {nKey, "set_antenna"}
    {nKey, "auto"}
    {nKey, "toggle"}
    {nKey, "short"}
    {nKey, "full"}
*/

const json_node_t msgDeviceInfo[] = {
    {nControl, "{"},                //
    {nKey, "device_info"},          //
    {nControl, "{"},                //
    {nNodeList, &message_ID},       //
    {nNodeList, &serial_number},    //
    {nKey, "name"},                 //
    {nString, &productName},        //
    {nKey, "software_version"},     //
    {nControl, "{"},                //
    {nNodeList, &software_version}, //
    {nControl, "}"},                //
    {nNodeList, &compilation_info}, //
    {nControl, "\e"},               //
};

const json_node_t deviceInfoShort[] = {
    {nControl, "{"},                //
    {nKey, "device_info"},          //
    {nControl, "{"},                //
    {nKey, "name"},                 //
    {nString, &productName},        //
    {nNodeList, &serial_number},    //
    {nControl, "\e"},               //
};

const json_node_t deviceInfoFull[] = {
    {nControl, "{"},                //
    {nKey, "device_info"},          //
    {nControl, "{"},                //
    {nKey, "name"},                 //
    {nString, &productName},        //
    {nNodeList, &serial_number},    //
    {nKey, "software_version"},     //
    {nControl, "{"},                //
    {nNodeList, &software_version}, //
    {nControl, "}"},                //
    {nNodeList, &compilation_info}, //
    {nControl, "\e"},               //
};

const json_node_t msgOk[] = {
    {nControl, "{"},          //
    {nKey, "ok"},             //
    {nControl, "{"},          //
    {nNodeList, &message_ID}, //
    {nControl, "\e"},         //
};

const json_node_t responseOk[] = {
    {nControl, "{"},    //
    {nKey, "response"}, //
    {nString, "ok"},    //
    {nControl, "\e"},   //
};

const json_node_t responseError[] = {
    {nControl, "{"},    //
    {nKey, "response"}, //
    {nString, "error"}, //
    {nControl, "\e"},   //
};

// This is the message ID recieved from the host computer
static char json_pongString[PONG_STRING_MAX_SIZE] = "?";

void set_json_pongString(char *string) {
    //
    strncpy(json_pongString, string, PONG_STRING_MAX_SIZE);
}

const json_node_t responsePong[] = {
    {nControl, "{"},            //
    {nKey, "pong"},             //
    {nString, &json_pongString}, //
    {nControl, "\e"},           //
};