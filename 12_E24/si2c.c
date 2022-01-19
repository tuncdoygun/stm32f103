#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "si2c.h"

enum { FALSE, TRUE };

void SI2C_Init(void)
{
  IO_Write(IOP_I2C_SCL, 1);
  IO_Init(IOP_I2C_SCL, IO_MODE_OUTPUT_OD);

  IO_Write(IOP_I2C_SDA, 1);
  IO_Init(IOP_I2C_SDA, IO_MODE_OUTPUT_OD);
}

static void I2C_Wait(void)
{
  //DelayUs(1);
}

static void SDA_Low(void)
{
  IO_Write(IOP_I2C_SDA, 0);
  
  I2C_Wait();
}

static void SDA_High(void)
{
  IO_Write(IOP_I2C_SDA, 1);
  
  I2C_Wait();
}

static void SCL_Low(void)
{
  IO_Write(IOP_I2C_SCL, 0);
  
  I2C_Wait();
}

static void SCL_High(void)
{
  IO_Write(IOP_I2C_SCL, 1);
  
  //while (!IO_Read(IOP_I2C_SCL)) ;     // Clock stretching
  
  I2C_Wait();
}

// Ön Koþul SCL=0 olmalý
// Çýkýþ koþulu: SCL=0 SDA=1
static void SI2C_SendBit(int b)
{
  // 1) Data setup
  if (b)
    SDA_High();
  else
    SDA_Low();
  
  // Data Setup Time
  // Gerekli olmayacak, çünkü low/high fonksiyonlarýnda bekleme var
      
  // 2) Clock generation
  SCL_High();
  SCL_Low();  
  
  SDA_High();
}

static int SI2C_RecvBit(void)
{
  int b;
  
  // 1) Clock generation (active)
  SCL_High();
  
  // 2) Sampling (örnekleme/okuma)
  b = IO_Read(IOP_I2C_SDA);
  
  // 3) Clock inactive
  SCL_Low();
  
  return b;
}

// I2C üzerinden 1-byte veri gönderir
// slave'in ACK cevabýna geri döner
static int SI2C_SendByte(unsigned char val)
{
  int i;
  
  for (i = 0; i < 8; ++i) {
    SI2C_SendBit(val & 0x80);
    val <<= 1;
  }
  
  return SI2C_RecvBit();
}

// I2C üzerinden 1-bte veri okur
// Geri dönüþ deðeri: okunan veri
static unsigned char SI2C_RecvByte(void)
{
  int i;
  unsigned char val;
  
  for (i = 0; i < 8; ++i) {
    val <<= 1;
    
    if (SI2C_RecvBit())
      val |= 1;        
  }
  
  return val;
}

// Start koþulu oluþturur ve kontrol byte'ý gönderir
// Ön koþul: SDA=1
// Çýkýþ koþulu: SCL=0 SDA=1
static int SI2C_Start(unsigned char ctl)
{
  SCL_High();  
  SDA_Low();
  
  SCL_Low();
  SDA_High();
  
  return SI2C_SendByte(ctl);
}

// Ön koþul: SDA=1, SCL=0
static void SI2C_Stop(void)
{
  SDA_Low();    // SCL=0, SDA=0
  
  SCL_High();   // SCL=1, SDA=0
  
  SDA_High();   // SCL=1, SDA=1 (idle)
}

int SI2C_Write(unsigned char devAdr, const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  devAdr &= 0xFE;       // 11111110 devAdr<0> = 0 (yazma)
  
  if (SI2C_Start(devAdr)) {
    SI2C_Stop();
    return FALSE;
  }
  
  while (len--) {
    if (SI2C_SendByte(*ptr++)) {
      SI2C_Stop();
      return FALSE;
    }
  }
  
  SI2C_Stop();
  return TRUE;  
}

int SI2C_Read(unsigned char devAdr, void *buf, int len)
{
  unsigned char *ptr = (unsigned char *)buf;
  int ack;
  
  if (len <= 0)
    return FALSE;
  
  if (SI2C_Start(devAdr | 1)) {
    SI2C_Stop();
    return FALSE;
  }
  
  do {
    *ptr++ = SI2C_RecvByte();
    
    ack = (len == 1);
    SI2C_SendBit(ack);    
  } while(--len);
  
  SI2C_Stop();
  return TRUE;
}





