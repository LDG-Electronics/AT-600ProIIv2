#include "includes.h"

/* ************************************************************************** */

void spi_init(void)
{
    SPI1CON1bits.SMP = 0; // SDI is sampled in the middle of data output time
    SPI1CON1bits.CKE = 0; // Output data changes on transition from idle to active clock state
    SPI1CON1bits.CKP = 0; // Idle state for SCK is low level
    SPI1CON1bits.FST = 0; // Delay to first SCK will be at least ½ baud period
    SPI1CON1bits.SSP = 0; // SS is active-high
    SPI1CON1bits.SDIP = 0; // SDI is active-high
    SPI1CON1bits.SDOP = 0; // SDO is active-high

    SPI1CON2bits.SSET = 1; // SS(out) is driven to the active state continuously
    SPI1CON2bits.TXR = 1; // TxFIFO data is required for a transfer
    SPI1CON2bits.RXR = 1; // Data transfers are suspended if the RxFIFO is full
    
    SPI1BAUD = 0x00; // BAUD 0, no prescaler is used 
    
    SPI1CLK = 0x00; // CLKSEL FOSC; 
    
    SPI1CON0bits.MST = 1; // Set as bus master
    SPI1CON0bits.EN = 1; // Enable SPI module
}

/* -------------------------------------------------------------------------- */

#define SPI_RX_IN_PROGRESS 0x0
#define DUMMY_DATA 0x0

void spi_tx()
{

}


uint8_t SPI1_Exchange8bit(uint8_t data)
{
    //One byte transfer count   
    SPI1TCNTL = 1;
    SPI1TXB = data;

    while(PIR2bits.SPI1RXIF == SPI_RX_IN_PROGRESS)
    {
    }

    return (SPI1RXB);
}

uint8_t SPI1_Exchange8bitBuffer(uint8_t *dataIn, uint8_t bufLen, uint8_t *dataOut)
{
    uint8_t bytesWritten = 0;

    if(bufLen != 0)
    {
        if(dataIn != NULL)
        {
            while(bytesWritten < bufLen)
            {
                if(dataOut == NULL)
                {
                    SPI1_Exchange8bit(dataIn[bytesWritten]);
                }
                else
                {
                    dataOut[bytesWritten] = SPI1_Exchange8bit(dataIn[bytesWritten]);
                }

                bytesWritten++;
            }
        }
        else
        {
            if(dataOut != NULL)
            {
                while(bytesWritten < bufLen )
                {
                    dataOut[bytesWritten] = SPI1_Exchange8bit(DUMMY_DATA);

                    bytesWritten++;
                }
            }
        }
    }

    return bytesWritten;
}