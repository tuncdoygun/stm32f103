#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"

#define HI2C_TIMEOUT    1000

static I2C_TypeDef     *_i2c[] = {
  I2C1,
  I2C2,
};

enum { FALSE, TRUE };

// Hardware I2C �evresel birimi ba�latma fonksiyonu
void HI2C_Init(int nI2C, int bRate)
{
  I2C_InitTypeDef i2cInit;
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  if (nI2C == I2C_1) {
    // 2) SCL ve SDA I/O u�lar� yap�land�r�lacak
    IO_Init(IOP_I2C1_SCL, IO_MODE_ALTERNATE_OD);
    IO_Init(IOP_I2C1_SDA, IO_MODE_ALTERNATE_OD);

    // 1) I2C �evresel birim clock aktif olmal�
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);    
  }
  else if (nI2C == I2C_2) {
    // 2) SCL ve SDA I/O u�lar� yap�land�r�lacak
    IO_Init(IOP_I2C2_SCL, IO_MODE_ALTERNATE_OD);
    IO_Init(IOP_I2C2_SDA, IO_MODE_ALTERNATE_OD);

    // 1) I2C �evresel birim clock aktif olmal�
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);
  }
  
  // 3) I2C ba�lang�� parametreleri yap�land�r�lacak
  i2cInit.I2C_Ack = I2C_Ack_Enable; // ??
  i2cInit.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  i2cInit.I2C_ClockSpeed = bRate;
  i2cInit.I2C_DutyCycle = I2C_DutyCycle_2;
  i2cInit.I2C_Mode = I2C_Mode_I2C;
  i2cInit.I2C_OwnAddress1 = 0x55;
  
  I2C_Init(pI2C, &i2cInit);
  
  // 4) �evresel birim aktif (on)
  I2C_Cmd(pI2C, ENABLE);
}

static int HI2C_Event(int nI2C, int event)
{
  unsigned long tmOut = HI2C_TIMEOUT;
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  do {
    if (I2C_CheckEvent(pI2C, event) == SUCCESS)
      break;
  } while (--tmOut);
  
  return (tmOut != 0);
}

// Stop ko�ulu olu�turur
void HI2C_Stop(int nI2C)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  I2C_GenerateSTOP(pI2C, ENABLE);
}

// Start ko�ulu olu�turur, ard�ndan konreol byte'� g�nderir
// Slave'in ACK cevab�na geri d�ner
// Geri d�n��: TRUE, FALSE
int HI2C_Start(int nI2C, unsigned char ctl)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  int stat;
  
  I2C_GenerateSTART(pI2C, ENABLE);      // Start condition
  
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_MODE_SELECT)) 
    return FALSE;
  
  I2C_SendData(pI2C, ctl);     // !!
  
  stat = HI2C_Event(nI2C, ctl & 1 ? 
    I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED :
    I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
  
  if (stat != SUCCESS)
    I2C_GenerateSTOP(pI2C, ENABLE);      // Stop condition
  
  return stat;
}
                      
// I2C bus �zerinden 8-bit veri g�nderir
// Geri d�n��: Slave acknowledge TRUE: ba�ar�l�
int HI2C_Send(int nI2C, unsigned char val)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];

  I2C_SendData(pI2C, val);
  
  return HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_TRANSMITTED);
}

int HI2C_Recv(int nI2C, unsigned char *pVal)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];
  
  if (!HI2C_Event(nI2C, I2C_EVENT_MASTER_BYTE_RECEIVED))
    return FALSE;
  
  *pVal = I2C_ReceiveData(pI2C);
  return TRUE;
}

// Slave'e ack g�nderme veya yap�land�rmas�
// ack: ACK biti lojik de�eri 0: olumlu
void HI2C_Acknowledge(int nI2C, int ack)
{
  I2C_TypeDef *pI2C = _i2c[nI2C];

  I2C_AcknowledgeConfig(pI2C, ack ? DISABLE : ENABLE);
}

////////////////////////////////////////////////////////////////////////////////
// GENERIC I2C FUNCTIONS

// I2C Generic Write
// I2C �zerinden yazma c�mlesi ile veri g�nderir
// nI2C: I2C �evresel no
// devAdr: Slave device hardware 7-bit address (high 7-bit)
// buf: G�nderilecek verinin ba�lang�� adresi
// len: G�nderilecek veri uzunlu�u (kontrol byte hari�)
int HI2C_Write(int nI2C, unsigned char devAdr, const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  if (len < 0)
    return FALSE;
  
  devAdr &= 0xFE;
  
  if (!HI2C_Start(nI2C, devAdr)) 
    return FALSE;
  
  while (len) {
    if (!HI2C_Send(nI2C, *ptr++)) 
      break;
    
    --len;    
  }
    
  HI2C_Stop(nI2C);  
  return !len;    
}

// I2C Generic Read
// I2C �zerinden okuma c�mlesi ile veri al�r
// nI2C: I2C �evresel no
// devAdr: Slave device hardware 7-bit address (high 7-bit)
// buf: Al�nacak verinin ba�lang�� adresi
// len: Al�nacakk veri uzunlu�u (kontrol byte hari�)
int HI2C_Read(int nI2C, unsigned char devAdr, void *buf, int len)
{
  unsigned char *ptr = (unsigned char *)buf;
  int ack;

  if (len <= 0)
    return FALSE;
  
  if (!HI2C_Start(nI2C, devAdr | 1)) 
    return FALSE;
  
  do {
    ack = (len == 1);
    HI2C_Acknowledge(nI2C, ack);
    
    if (!HI2C_Recv(nI2C, ptr++))
      break;
  } while (--len);
  
  HI2C_Stop(nI2C);
  return !len;
}

////////////////////////////////////////////////////////////////////////////////

#define SZ_I2CBUF       256

// 1 Byte register adres kullanan device'lar i�in yazma
int HI2C_WriteA(int nI2C, unsigned char devAdr, unsigned char regAdr, const void *buf, int len)
{
  unsigned char i2cbuf[SZ_I2CBUF + 1];
  
  if (len > SZ_I2CBUF)
    return FALSE;

  i2cbuf[0] = regAdr;
  memcpy(i2cbuf + 1, buf, len);
  
  return HI2C_Write(nI2C, devAdr, i2cbuf, len + 1);
}

// 1 Byte register adres kullanan device'lar i�in okuma
int HI2C_ReadA(int nI2C, unsigned char devAdr, unsigned char regAdr, void *buf, int len)
{
  if (!HI2C_Write(nI2C, devAdr, &regAdr, 1))
    return FALSE;
  
  if (!HI2C_Read(nI2C, devAdr, buf, len))
    return FALSE;
  
  return TRUE;
}


