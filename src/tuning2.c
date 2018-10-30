#include "tuning2.h"
#include "calibration.h"
#include "display.h"
#include "flags.h"
#include "memory.h"
#include "os/log_macros.h"
#include "os/system_time.h"
#include "relays.h"
#include "rf_sensor.h"
static uint8_t LOG_LEVEL = L_SILENT;
/* ************************************************************************** */

void tuning2_init(void) { log_register(); }