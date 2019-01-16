#ifndef _SHELL_COMMANDS_H_
#define _SHELL_COMMANDS_H_

#include "sh_adc.h"
#include "sh_bar.h"
#include "sh_eeprom.h"
#include "sh_flash.h"
#include "sh_logedit.h"
#include "sh_poly.h"
#include "sh_relays.h"
#include "sh_romedit.h"
#include "sh_status.h"
#include "sh_tune.h"

/* ************************************************************************** */

#define SHELL_COMMANDS                                                         \
    SH_ADC, SH_BAR, SH_EEPROM, SH_FLASH, SH_LOGEDIT, SH_POLY, SH_RELAYS,       \
        SH_ROMEDIT, SH_STATUS, SH_TUNE

/* ************************************************************************** */

#endif // _SHELL_COMMANDS_H_