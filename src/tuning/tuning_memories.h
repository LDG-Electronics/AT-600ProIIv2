#ifndef _TUNING_MEMORIES_H_
#define _TUNING_MEMORIES_H_

#include "relays.h"
#include <stdint.h>

/* ************************************************************************** */

// setup
extern void tuning_memories_init(void);

/* ************************************************************************** */

// Return the memory slot associated with the given frequency
extern uint16_t find_memory_slot(uint16_t frequency);

// Recall
extern relays_t recall_memory(uint16_t slot);

// Store
extern void store_memory(uint16_t slot, relays_t relays);

#endif // _TUNING_MEMORIES_H_