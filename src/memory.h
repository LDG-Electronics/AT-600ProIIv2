#ifndef _MEMORY_H_
#define _MEMORY_H_

/* ************************************************************************** */

// Converts a frequency into an address in memory.
extern uint24_t convert_memory_address(uint16_t frequency);

// Store the current relay settings at the provided address
extern void memory_store(uint24_t address);

// Recall relay settings from the provided address
extern uint32_t memory_recall(uint24_t address);

#endif