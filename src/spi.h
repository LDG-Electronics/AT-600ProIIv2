#ifndef _SPI_H_
#define _SPI_H_

/* ************************************************************************** */

extern void spi_init(void);

/* -------------------------------------------------------------------------- */

extern uint8_t SPI1_Exchange8bit(uint8_t data);
extern uint8_t SPI1_Exchange8bitBuffer(uint8_t *dataIn, 
                                       uint8_t bufLen,
                                       uint8_t *dataOut);

#endif /* _SPI_H_ */
