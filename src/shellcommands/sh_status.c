// #include "flags.h"
// #include "os/serial_port.h"
// #include "os/shell/shell_command_processor.h"
// #include "os/shell/shell_json.h"
// #include "rf_sensor.h"
// #include <ctype.h>
// #include <stdlib.h>
// #include <string.h>

// /* ************************************************************************** */

// //
// const json_field_t JSONcurrentFlags[] = {
//     {"bypassStatus[0]", &systemFlags.bypassStatus[0], jsonU8}, //
//     {"bypassStatus[1]", &systemFlags.bypassStatus[1], jsonU8}, //
//     {"antenna", &systemFlags.antenna, jsonU8},                 //
//     {"autoMode", &systemFlags.autoMode, jsonU8},               //
//     {"peakMode", &systemFlags.peakMode, jsonU8},               //
//     {"scaleMode", &systemFlags.scaleMode, jsonU8},             //
//     {"powerStatus", &systemFlags.powerStatus, jsonU8},         //
//     {NULL, NULL, jsonObject},                                  //
// };

// /* -------------------------------------------------------------------------- */

// const json_field_t JSONcurrentRF[] = {
//     {"forward", &(currentRF.forward), jsonFloat},           //
//     {"reverse", &(currentRF.reverse), jsonFloat},           //
//     {"matchQuality", &(currentRF.matchQuality), jsonFloat}, //
//     {"forwardWatts", &(currentRF.forwardWatts), jsonFloat}, //
//     {"reverseWatts", &(currentRF.reverseWatts), jsonFloat}, //
//     {"swr", &(currentRF.swr), jsonFloat},                   //
//     {"frequency", &(currentRF.frequency), jsonU16},         //
//     {NULL, NULL, jsonObject},                               //
// };

// /* -------------------------------------------------------------------------- */

// void sh_status(int argc, char **argv) {
//     switch (argc) {
//     case 1: // usage
//         print("usage: ");
//         println("\tstatus rf");
//         println("\tstatus flags");
//         return;
//     case 2:
//         if (!strcmp(argv[1], "rf")) { // status rf
//             json_serialize_and_print(&JSONcurrentRF[0]);
//             println("");
//             return;
//         } else if (!strcmp(argv[1], "flags")) { // status flags
//             json_serialize_and_print(&JSONcurrentFlags[0]);
//             println("");
//             return;
//         }
//         break;
//     }
//     println("invalid arguments");
// }

// REGISTER_SHELL_COMMAND(sh_status, "status");