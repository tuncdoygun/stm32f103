#ifndef _SPI_H
#define _SPI_H

void SSPI_Start(void);
uint8_t SSPI_Data(uint8_t val);

void HSPI_Start(void);
uint8_t HSPI_Data(uint8_t val);

#define SPI_Start       HSPI_Start
#define SPI_Data        HSPI_Data

#endif