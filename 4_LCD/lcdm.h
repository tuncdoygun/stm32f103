#ifndef _LCDM_H
#define _LCDM_H

#define LCD_MODE_BLINK  1
#define LCD_MODE_CURSOR 2
#define LCD_MODE_ON     4

void LCD_Init(void);
void LCD_PutChar(unsigned char c);
void LCD_SetCursor(unsigned int pos);
void LCD_DisplayOn(unsigned char mode);
void LCD_putch(unsigned char c);

#endif