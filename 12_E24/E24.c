#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "system.h"
#include "io.h"
#include "si2c.h"
#include "E24.h"

#define I2C_ADR_24XX    0xA0
#define I2CN_E24        I2C_2

#define PGSIZEMAX       256
#define ADSIZEMAX         4

static int _AdSize = 2;
static int _PgSize = 64;

static unsigned char _I2CBuf[PGSIZEMAX + ADSIZEMAX];

enum { FALSE, TRUE };

void E24_Config(int adSize, int pgSize)
{
  if (adSize <= ADSIZEMAX)
    _AdSize = adSize;

  if (pgSize <= PGSIZEMAX)
    _PgSize = pgSize;
}

int E24_WritePage(int chipAdr, unsigned long offset, const void *buffer, int size)
{
  int i;
  
  if (size > _PgSize)
    return FALSE;
  
  chipAdr <<= 1;
  
  for (i = _AdSize - 1; i >= 0; --i) {
    _I2CBuf[i] = (unsigned char)offset;
    offset >>= 8;
  }
  
  memcpy(_I2CBuf + _AdSize, buffer, size);
  
  //if (!HI2C_Write(I2CN_E24, I2C_ADR_24XX + chipAdr, _I2CBuf, size + _AdSize))
  if (!SI2C_Write(I2C_ADR_24XX + chipAdr, _I2CBuf, size + _AdSize))
    return FALSE;
  
  //DelayMs(3);
  
  // Acknowledge polling
  //while (!HI2C_Write(I2CN_E24, I2C_ADR_24XX + chipAdr, NULL, 0)) ;
  while (!SI2C_Write(I2C_ADR_24XX + chipAdr, NULL, 0)) ; // cihaz cevap verene kadar bekliyor.
  
  return TRUE;
}

int E24_Write(int chipAdr, unsigned long offset, const void *buffer, int size)
{
  int bSize;
  const unsigned char *ptr = (const unsigned char *)buffer;
  
  bSize = _PgSize - (offset % _PgSize);
  
  while (size > 0) {
    if (bSize > size)
      bSize = size;
    
    if (!E24_WritePage(chipAdr, offset, ptr, bSize))
      return FALSE;
    
    offset += bSize;
    ptr += bSize;
    size -= bSize;
    
    bSize = _PgSize;
  }
  
  return TRUE;
}

int E24_Read(int chipAdr, unsigned long offset, void *buffer, int size)
{
  int i;

  for (i = _AdSize - 1; i >= 0; --i) {
    _I2CBuf[i] = (unsigned char)offset;
    offset >>= 8;
  }
  
  chipAdr <<= 1;
  
  //if (!HI2C_Write(I2CN_E24, I2C_ADR_24XX + chipAdr, _I2CBuf, _AdSize))
  if (!SI2C_Write(I2C_ADR_24XX + chipAdr, _I2CBuf, _AdSize))
    return FALSE;
  
  //if (!HI2C_Read(I2CN_E24, I2C_ADR_24XX + chipAdr, buffer, size))
  if (!SI2C_Read(I2C_ADR_24XX + chipAdr, buffer, size))  
    return FALSE;
  
  return TRUE;
}
