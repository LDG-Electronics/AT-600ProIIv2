#ifndef _MEMORY_H_
#define _MEMORY_H_

/* ************************************************************************** */

// Converts a traditional style perioud into an address in memory.
extern uint24_t convert_memory_address(uint16_t period);

extern void memory_store(uint24_t address);
extern uint32_t memory_recall(uint24_t address);

#endif