#ifndef _MEMORY_H_
#define _MEMORY_H_

/* ************************************************************************** */

// Converts a traditional style perioud into an address in memory.
extern uint32_t convert_memory_address(uint16_t period);

extern void memory_store(uint32_t address);
extern relays_s memory_recall(uint32_t address);

#endif