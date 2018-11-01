#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "peripherals/nonvolatile_memory.h"
#include "relays.h"

/* ************************************************************************** */

// setup
extern void memory_init(void);

// converts the frequenct(in KHz) to a memory address
extern NVM_address_t convert_memory_address(uint16_t frequency);

// Recall relay settings from the provided address
extern relays_t memory_recall(NVM_address_t address);

// Store the current relay settings at the provided address
extern void memory_store(NVM_address_t address, relays_t *relays);

#endif