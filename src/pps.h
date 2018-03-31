#ifndef _PPS_H_
#define	_PPS_H_

/* ************************************************************************** */

// Input macros
#define PPS_PORT_A 0b000111
#define PPS_PORT_B 0b001111
#define PPS_PORT_C 0b010111
#define PPS_PORT_D 0b011111
#define PPS_PORT_E 0b100111
#define PPS_PORT_F 0b101111

#define PPS_PIN_0 0b111000
#define PPS_PIN_1 0b111001
#define PPS_PIN_2 0b111010
#define PPS_PIN_3 0b111011
#define PPS_PIN_4 0b111100
#define PPS_PIN_5 0b111101
#define PPS_PIN_6 0b111110
#define PPS_PIN_7 0b111111

// Output macros
#define PPS_SPI1_SS 0b100000
#define PPS_SPI1_SDO 0b011111
#define PPS_SPI1_SCK 0b011110

#define PPS_UART2_RTS 0b011000
#define PPS_UART2_TXDE 0b010111
#define PPS_UART2_TX 0b010110
#define PPS_UART1_RTS 0b010101
#define PPS_UART1_TXDE 0b010100
#define PPS_UART1_TX 0b010011

#define PPS_CLC4OUT 0b000100
#define PPS_CLC3OUT 0b000011
#define PPS_CLC2OUT 0b000010
#define PPS_CLC1OUT 0b000001


#endif	/* _PPS_H_ */