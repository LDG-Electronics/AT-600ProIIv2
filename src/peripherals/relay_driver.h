#ifndef _RELAY_DRIVER_H_
#define _RELAY_DRIVER_H_

#include <stdint.h>

/* ************************************************************************** */

extern void relay_driver_init(void);

extern void publish_relays(uint16_t word);

#endif