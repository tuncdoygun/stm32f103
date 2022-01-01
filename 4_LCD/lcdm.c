#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "lcdm.h"

// Data: Saf veri (komut yada karakter kodu)
// Komut: RS=0
// Karakter: RS=1

// LCD Module 4-bit senkron veri gonderir.
// veri: karakter kodu veya komut kodu
// Gonderilecek veri parametrenin dusuk anlamli 4-bitinde
static void LCD_SendDataL(unsigned char c)
{
  // 1 - Data setup (verinin hazirlanmasi)
  IO_Write(IOP_LCD_DB4, (c & 1) != 0);
  IO_Write(IOP_LCD_DB5, (c & 2) != 0);
  IO_Write(IOP_LCD_DB6, (c & 4) != 0);
  IO_Write(IOP_LCD_DB7, (c & 8) != 0);
  
  // 2 - Clock Generation
  IO_Write(IOP_LCD_E, 1);
  DelayUs(3);
  IO_Write(IOP_LCD_E, 0);
  DelayUs(100);
}

// LCD Module 4-bit senkron komut gonderir.
static void LCD_SendCmdL(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 0); // RS=0 komut gonderim
  
  LCD_SendDataL(c);
}

// LCD Module 8-bit veri gonderir
// 2 Adýmda, once yuksek 4-bit, sonra dusuk 4-bit
void LCD_SendData(unsigned char c)
{
  LCD_SendDataL(c >> 4); 
  LCD_SendDataL(c);
}

// LCD Module 8-bit komut gonderir
// 2 Adýmda, once yuksek 4-bit, sonra dusuk 4-bit
void LCD_SendCmd(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 0); // RS=0 komut gonderim
  
  LCD_SendData(c);
}

// LCD Module 8-bit karakter gonderir
// 2 Adýmda, once yuksek 4-bit, sonra dusuk 4-bit
void LCD_PutChar(unsigned char c)
{
  IO_Write(IOP_LCD_RS, 1); // RS=0 komut gonderim
  
  LCD_SendData(c);
}

// Ekraný temizle
void LCD_Clear(void)
{
  LCD_SendCmd(0x01); // ilk 0 sonra 1 gönderecek. Clear display
  DelayMs(5);
}

void LCD_DisplayOn(unsigned char mode)
{
  LCD_SendCmd(0x08 | mode); // 0000 1xxx register ayarý: mode
}

void LCD_SetCursor(unsigned int pos)
{
  LCD_SendCmd(0x80 | pos); // 1xxx xxxx 
}

void LCD_Init(void)
{
  IO_Init(IOP_LCD_RS, IO_MODE_OUTPUT);
  IO_Write(IOP_LCD_E, 0); // E pini kritik olduðundan 0 olarak baþlamalý.
  IO_Init(IOP_LCD_E, IO_MODE_OUTPUT);
  
  IO_Init(IOP_LCD_DB4, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB5, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB6, IO_MODE_OUTPUT);
  IO_Init(IOP_LCD_DB7, IO_MODE_OUTPUT);
  
  // HD44 dökümanýnda sayfa 47
  DelayMs(100);
  // 4-bit arayüz ile baþlatma - HD44 dökümanýnda sayfa 47
  LCD_SendCmdL(0x03); // 4 - bit komut
  DelayMs(5);
  LCD_SendCmdL(0x03); // 4 - bit komut 100 us bunun içinde
  
  LCD_SendCmd(0x32); // ilk 3 sonra 2 gönderecek. 4 bit 4 bit
  LCD_SendCmd(0x28); // ilk 2 sonra 8 gönderecek. N=1 (iki satýr) F=0 (küçük font)
 
  LCD_DisplayOn(0); // Display off, ekrana perde olacakmýþ gibi
  LCD_Clear();
 
  LCD_SendCmd(0x06); // I/D = inc, dec.Her gönderimden sonra artma mý azalma mý yapýlsýn. I/D=1 (cursor artma)
                     // S=0
  LCD_DisplayOn(LCD_MODE_ON);
}

void LCD_putch(unsigned char c)
{
  switch(c){
  case '\r':
    LCD_SetCursor(0);
    break;
    
  case '\n':
    LCD_SetCursor(0x40);
    break;
    
  case '\f':
    LCD_Clear();
    break;
    
  default:
    LCD_PutChar(c);
    break;
  }
}