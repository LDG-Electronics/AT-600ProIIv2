#ifndef _SHELL_COMMANDS_H_
#define _SHELL_COMMANDS_H_

#include "sh_display.h"
#include "sh_eeprom.h"
#include "sh_flash.h"
#include "sh_logedit.h"
#include "sh_relays.h"
#include "sh_rf_sensor.h"
#include "sh_tune.h"

/* ************************************************************************** */

#define SHELL_COMMANDS                                                         \
    SH_CAL, SH_TUNE, SH_LOGEDIT, SH_RELAYS, SH_EEPROM, SH_FLASH, SH_BAR

/* ************************************************************************** */

#endif