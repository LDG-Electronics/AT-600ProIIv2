#ifndef _SPI_H_
#define _SPI_H_

/* ************************************************************************** */

extern void spi_init(void);

/* -------------------------------------------------------------------------- */

// Transmits two bytes of data, specifically for the ProII series front panel
extern void spi_tx_word(uint16_t data);

// Transmits a string 
extern void spi_tx_string(const char *string, uint16_t length);

#endif