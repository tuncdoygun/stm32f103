#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_spi.h"
#include "system.h"
#include "io.h"

////////////////////////////////////////////////////////////////////////////////
// SOFTWARE SPI

#define SSPI_Delay()    __NOP();
/*
void SSPI_Delay(void)
{
}
*/
void SSPI_Start(void)
{
  // SPI pinleri yapýlandýrýlacak
  IO_Init(IOP_SPI_SCK, IO_MODE_OUTPUT);
  IO_Init(IOP_SPI_MOSI, IO_MODE_OUTPUT);
  IO_Init(IOP_SPI_MISO, IO_MODE_INPUT);
}

// SPI birimi üzerinden 8-bit veri gönderir ve alýr
// val: gönderilen deðer
// return: alýnan deðer
// mode 0 (idle: 0, örnekleme ilk kenarda - yükselen)
uint8_t SSPI_Data(uint8_t val)
{
  int i;
  
  for (i = 0; i < 8; ++i) {
    // 1) Data setup (gönderilecek verinin hazýrlanmasý)
    // MSB first gönderme ve alma
    // Gönderilecek deðer en yüksek anlamlý bitte (b7)
    IO_Write(IOP_SPI_MOSI, val & 0x80 ? 1 : 0);
    val <<= 1; // val = val << 1;
    SSPI_Delay();       // Data setup time
    
    // 2) Clock generation (onay saat iþareti üretimi)
    IO_Write(IOP_SPI_SCK, 1);   // SCK idle -> active
    SSPI_Delay();       // Clock high time
    
    // 3) Read data - sampling (gelen verinin okunmasý)
    if (IO_Read(IOP_SPI_MISO))
      val |= 1;

    IO_Write(IOP_SPI_SCK, 0);   // SCK active -> idle
    SSPI_Delay();
  }
  
  return val;
}

////////////////////////////////////////////////////////////////////////////////
// HARDWARE SPI

#define SPI_PORT        SPI1

// Donanýmsal SPI çev. birimini baþlatýr
void HSPI_Start(void)
{
  SPI_InitTypeDef spiInit;
  
  // 1) SPI clock aktif olacak
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  // 2) SPI pinleri yapýlandýrýlacak
  IO_Init(IOP_SPI_SCK, IO_MODE_ALTERNATE);
  IO_Init(IOP_SPI_MOSI, IO_MODE_ALTERNATE);
  IO_Init(IOP_SPI_MISO, IO_MODE_INPUT);
  
  // 3) SPI Init structure parametreleri yapýlandýrýlacak
  spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  // SPI mode 0
  spiInit.SPI_CPOL = SPI_CPOL_Low;
  spiInit.SPI_CPHA = SPI_CPHA_1Edge;
  spiInit.SPI_DataSize = SPI_DataSize_8b;
  spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInit.SPI_Mode = SPI_Mode_Master;
  spiInit.SPI_NSS = SPI_NSS_Soft;
  
  SPI_Init(SPI_PORT, &spiInit);
  
  // 4) SPI çevresel birimini baþlat
  SPI_Cmd(SPI_PORT, ENABLE);
}

// SPI birimi üzerinden 8-bit veri gönderir ve alýr
// val: gönderilen deðer
// return: alýnan deðer
uint8_t HSPI_Data(uint8_t val)
{
  // SPI Transmit buffer boþ mu?
  while (!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_TXE)) ;
  
  SPI_I2S_SendData(SPI_PORT, val);
  
  while (!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_RXNE)) ;
  
  val = SPI_I2S_ReceiveData(SPI_PORT);
  
  return val;
}

