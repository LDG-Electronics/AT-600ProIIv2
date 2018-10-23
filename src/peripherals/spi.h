#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>

/* ************************************************************************** */

// setup
extern void spi_init(void);

/* -------------------------------------------------------------------------- */

// Transmits two bytes of data, specifically for the ProII series front panel
extern void spi_tx_word(uint16_t data);

#endif