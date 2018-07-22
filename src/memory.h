#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "peripherals/nonvolatile_memory.h"

/* ************************************************************************** */

// Converts a frequency into an address in memory.
extern NVM_address_t convert_memory_address(uint16_t frequency);

// Store the current relay settings at the provided address
extern void memory_store(NVM_address_t address);

// Recall relay settings from the provided address
extern uint32_t memory_recall(NVM_address_t address);

#endif