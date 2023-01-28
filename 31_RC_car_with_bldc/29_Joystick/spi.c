#include <stdio.h>
#include <stdint.h>

#include "stm32f10x_spi.h"
#include "system.h"
#include "io.h"
#include "spi.h"

struct SPI_CFG {
  int ioSck;
  int ioMosi;
  int ioMiso;
  
  uint32_t clkSPI; // clock SPI
  SPI_TypeDef *pSPI;
};

static struct SPI_CFG _uCfg[] = {
  {IOP_SPI_SCK, IOP_SPI_MOSI, IOP_SPI_MISO, RCC_APB2Periph_SPI1, SPI1},
  {IOP_SPI2_SCK, IOP_SPI2_MOSI, IOP_SPI2_MISO, RCC_APB1Periph_SPI2, SPI2},
};

// Donan�msal SPI �ev. birimini ba�lat�r.
void SPI_Start(int idx) 
{
  SPI_InitTypeDef spiInit;
  
  // 1 - SPI Clock aktif edilir.
  if (idx == SPI_1)
    RCC_APB2PeriphClockCmd(_uCfg[idx].clkSPI, ENABLE);
  else
    RCC_APB1PeriphClockCmd(_uCfg[idx].clkSPI, ENABLE);
  
  // 2 - SPI pinleri yap�land�r�l�r.   
  IO_Init(_uCfg[idx].ioSck, IO_MODE_ALTERNATE); // gpio port yerine spi'a devrediyoruz portu
  IO_Init(_uCfg[idx].ioMosi, IO_MODE_ALTERNATE);
  IO_Init(_uCfg[idx].ioMiso, IO_MODE_INPUT);    
    
  // 3 - SPI Init structure parametreleri yap�land�r�l�r.
  spiInit.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_64; // 18 MHz .spi �al��t�r�rken,spi �u h�zda �al��s�n demek yerine apb ka�a b�l�ns�n de h�z� belirlensin olarak hesaplan�r.
  spiInit.SPI_CPOL = SPI_CPOL_Low; // Mode - 0
  spiInit.SPI_CPHA = SPI_CPHA_1Edge;
  spiInit.SPI_DataSize = SPI_DataSize_8b;
  spiInit.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  spiInit.SPI_FirstBit = SPI_FirstBit_MSB;
  spiInit.SPI_Mode = SPI_Mode_Master;
  spiInit.SPI_NSS = SPI_NSS_Soft; // hardware olarak da kullan�labiliyor.

  SPI_Init(_uCfg[idx].pSPI, &spiInit);
  
  // 4 - SPI �evresel birimi ba�lat�l�r. 
  SPI_Cmd(_uCfg[idx].pSPI, ENABLE);    
}

// SPI �zerinden 8 bit veri g�nderir ve al�r.
// sadece veri almak i�in dummy bir de�er g�nderilebilir.
uint8_t SPI_Data(int idx, uint8_t val)
{  
  // SPI Transmit buffer bo� mu ?
  while(!SPI_I2S_GetFlagStatus(_uCfg[idx].pSPI, SPI_I2S_FLAG_TXE));
  
  SPI_I2S_SendData(_uCfg[idx].pSPI, val);
  
  while(!SPI_I2S_GetFlagStatus(_uCfg[idx].pSPI, SPI_I2S_FLAG_RXNE)); // normalde almadan ��kmaz gibi dursa da,tx ile rx ayn� anda �al���yor.
  
  val = SPI_I2S_ReceiveData(_uCfg[idx].pSPI);

  return val;
}
