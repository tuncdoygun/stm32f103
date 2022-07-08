#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_spi.h"
#include "system.h"
#include "io.h"

#define SPI_PORT        SPI1

// HARDWARE SPI

// Donan�msal SPI �ev. birimini ba�lat�r.
void SPI_Start(void) // ba�taki H hardware
{
  SPI_InitTypeDef spiInit;
  
  // 1 - SPI Clock aktif edilir.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
  
  // 2 - SPI pinleri yap�land�r�l�r.
  IO_Init(IOP_SPI_SCK, IO_MODE_ALTERNATE); // gpio port yerine spi'a devrediyoruz portu
  IO_Init(IOP_SPI_MOSI, IO_MODE_ALTERNATE);
  IO_Init(IOP_SPI_MISO, IO_MODE_INPUT);
  
  // 3 - SPI Init structure parametreleri yap�land�r�l�r.
  spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4; // 18 MHz .spi �al��t�r�rken,spi �u h�zda �al��s�n demek yerine apb ka�a b�l�ns�n de h�z� belirlensin olarak hesaplan�r.
  spiInit.SPI_CPOL = SPI_CPOL_Low; // Mode - 0
  spiInit.SPI_CPHA = SPI_CPHA_1Edge;
  spiInit.SPI_DataSize = SPI_DataSize_8b;
  spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInit.SPI_Mode = SPI_Mode_Master;
  spiInit.SPI_NSS = SPI_NSS_Soft; // hardware olarak da kullan�labiliyor.

  SPI_Init(SPI_PORT, &spiInit);
  
  // 4 - SPI �evresel birimi ba�lat�l�r.
  SPI_Cmd(SPI_PORT, ENABLE);
}

// SPI �zerinden 8 bit veri g�nderir ve al�r.
// sadece veri almak i�in dummy bir de�er g�nderilebilir.
uint8_t SPI_Data(uint8_t val)
{
  // SPI Transmit buffer bo� mu ?
  while(!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_TXE));
  
  SPI_I2S_SendData(SPI_PORT, val);
  
  while(!SPI_I2S_GetFlagStatus(SPI_PORT, SPI_I2S_FLAG_RXNE)); // normalde almadan ��kmaz gibi dursa da,tx ile rx ayn� anda �al���yor.
  
  val = SPI_I2S_ReceiveData(SPI_PORT);
  
  return val;
}
