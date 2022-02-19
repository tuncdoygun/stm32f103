#include <stdio.h>

#include "system.h"
#include "io.h"
#include "1-wire.h"
#include "crc8.h"

enum { FALSE, TRUE };

#define OW_SetByte(n)   OW_Byte(n)
#define OW_GetByte()   OW_Byte(0xFF) // veri okurken OW_Bit'e 1 degerini gondermek gerekir.veri gönderme-alma ayný zamanda yapýldýðýndan sadece veri almak için

void OW_Init(void)
{
  // DQ hattýný open-drain 1 (High-Z) olarak baþlatacaðýz
  IO_Write(IOP_DQ, 1);
  IO_Init(IOP_DQ, IO_MODE_OUTPUT_OD);
}

// ds18b20 datasheet sayfa 15'deki senaryo
// 1-Wire bus reset pulse oluþturur, slave(s) presence pulse
// cevabýna geri döner
// TRUE: Slave var ve hazýr
// FALSE: Slave yok ya da hazýr deðil
int OW_Reset(void)
{
  int tmOut;
  
  Sys_EnterCritical();  // Kesmeler kapalý
  
  IO_Write(IOP_DQ, 0);  // DQ = 0
  DelayUs(500);         // Reset pulse
  
  // Hattý býrakacaðýz
  IO_Write(IOP_DQ, 1);  // DQ = High-Z -> pull-up 1
  
  DelayUs(10);
  
  tmOut = 250;
  do {
    if (!IO_Read(IOP_DQ))
      break;
    
    DelayUs(1);
  } while (--tmOut);
  
  if (tmOut == 0)
    goto END;
  
  tmOut = 250;
  do {
    if (IO_Read(IOP_DQ))
      break;
    
    DelayUs(1);
  } while (--tmOut);
  
END:
  Sys_ExitCritical();
  return (tmOut != 0);
}

// 1-Wire bus üzerinden 1-bit veri gönderir
// Master örnekleme deðerine (okuma) geri döner
int OW_Bit(int val)
{
  Sys_EnterCritical();
  
  IO_Write(IOP_DQ, 0);  // DQ = 0
  DelayUs(1);
  
  if (val)
    IO_Write(IOP_DQ, 1); // DQ = High-Z
  
  DelayUs(13);
  
  val = IO_Read(IOP_DQ); // Master samples
  
  DelayUs(46); // toplam 60 us beklemek gerektiginden
  
  IO_Write(IOP_DQ, 1); // DQ = High-Z
  
  DelayUs(1);
  
  Sys_ExitCritical();
  return val;
}

unsigned char OW_Byte(unsigned char val)
{
  int i, b;
  
  for (i = 0; i < 8; ++i) {
    b = OW_Bit(val & 1);
    val >>= 1;
    
    if (b) // slave'den gelen bit kontrolu.
      val |= 0x80;
  }
  
  return val;
}

// ROM seri numarasini okur
int DS_ReadROM(unsigned char *buf)
{
  int i;
  
  if (!OW_Reset())
    return FALSE;
  
  OW_SetByte(0x33);     // Read ROM
  
  // Slave 8-byte unique ID gönderir
  for (i = 0; i < 8; ++i) 
    buf[i] = OW_GetByte();
  
  OW_Reset();
  return TRUE;
}

//////////////////////////////////////////////////////////////

// Koþul: Bus üzerinde tek DS18B20 var
int DS_ReadTemp(int *pTemp)
{
  unsigned char sp[9]; // scratchpad.ilk 2 byte sicaklik, 9. byte CRC
  int i, tmOut;
  
  if (pTemp == NULL)
    return FALSE;
  
  //////////////////////////////////////
  // 1. cümle
  
  if (!OW_Reset())
    return FALSE;
  
  // ROM komutu
  OW_SetByte(0xCC);     // Skip ROM

  // Ýþlev komutu
  OW_SetByte(0x44);     // Convert T
  
  tmOut = 1000;
  
  do {
    if (IO_Read(IOP_DQ))
      break;
  } while (--tmOut);
  
  if (tmOut == 0) {
    OW_Reset(); // Gerekli olmayabilir
    return FALSE;
  }
  
  //////////////////////////////////////
  // 2. cümle
  
  if (!OW_Reset())
    return FALSE;
  
  // ROM komutu
  OW_SetByte(0xCC);     // Skip ROM

  // Ýþlev komutu
  OW_SetByte(0xBE);     // Read scratchpad
  
  for (i = 0; i < 9; ++i)
    sp[i] = OW_GetByte();
  
  OW_Reset();
  
  // Hata denetleme
  if (CRC8(sp, 9))
    return FALSE;
  
  *pTemp = (int)(*(int16_t *)sp);
  return TRUE;
}

//////////////////////////////////////////////////////////////////

#define TM_RESET        100
#define TM_CONVERT      1000

int Task_Thermo(void)
{
  static enum {
    S_INIT,
    S_RESET_1,
    S_WAIT_1,
    S_CONVERT,
    S_WAIT_C,
    S_RESET_2,
    S_READ_CMD,
    S_READ,
    S_CHECK,
  } state = S_INIT;
  
  static clock_t t0, t1;
  static int i, temp;
  static unsigned char sp[9];
  
  t1 = clock();
  
  switch (state) {
  case S_INIT:
    temp = TEMP_INVALID;
    t0 = t1;
    state = S_RESET_1;
    break;
    
  case S_RESET_1:
    if (OW_Reset())
      state = S_CONVERT;
    else {
      t0 = t1;
      state = S_WAIT_1;
    }
    break;
    
  case S_WAIT_1:
    if (t1 - t0 >= TM_RESET)
      state = S_RESET_1;
    break;
    
  case S_CONVERT:
    // Skip ROM komutu
    OW_SetByte(0xCC);
    
    // Ýþlev komutu
    OW_SetByte(0x44);   // Convert T
    t0 = t1;
    state = S_WAIT_C;
    break;
    
  case S_WAIT_C:
    if (IO_Read(IOP_DQ))
      state = S_RESET_2;
    else if (t1 - t0 >= TM_CONVERT)
      state = S_INIT;
    break;
    
  case S_RESET_2:
    if (OW_Reset())
      state = S_READ_CMD;
    else
      state = S_INIT;
    break;
        
  case S_READ_CMD:
    // Skip ROM komutu
    OW_SetByte(0xCC);
    
    // Ýþlev komutu
    OW_SetByte(0xBE);   // Read SP
    
    i = 0;
    t0 = t1;
    state = S_READ;
    break;
    
  case S_READ:
    sp[i++] = OW_GetByte();
    sp[i++] = OW_GetByte();
    sp[i++] = OW_GetByte();
    
    if (i >= 9)
      state = S_CHECK;
    break;
    
  case S_CHECK:
    // Error check (CRC)
    if (!CRC8(sp, 9)) // Scratchpad'deki ilk 9 byte CRC'ye gonderilirse 0 doner.
      temp = (int)(*(int16_t *)sp);
      
    state = S_INIT;
    break;
  }
  
  return temp;
}