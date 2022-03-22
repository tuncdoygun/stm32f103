#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"
#include "ertc.h"

#define I2C_PORT_ERTC   I2C_1
#define I2C_ADR_DS3231  0xD0

void ERTC_GetDate(ERTC_DATE *pdt, int regAdr, int len)
{
  HI2C_ReadA(I2C_PORT_ERTC, I2C_ADR_DS3231, regAdr, 
             (uint8_t *)pdt + regAdr , len);
}

void ERTC_SetDate(ERTC_DATE *pdt, int regAdr, int len)
{
  HI2C_WriteA(I2C_PORT_ERTC, I2C_ADR_DS3231, regAdr, 
              (uint8_t *)pdt + regAdr , len);
}

void ERTC_Init(void)
{
  ERTC_DATE dt;
  
  HI2C_Init(I2C_PORT_ERTC, 400000);
  
  dt.ctl = 0;
  ERTC_SetDate(&dt, 0x0E, 1);
}
