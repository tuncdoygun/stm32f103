#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "lcdm.h"
#include "uart.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1); // 0 olarak baþlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  //LCDM Baþlangýç
  //LCD_Init();
  
  // Konsol Baþlangýç
  Sys_ConsoleInit();
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,// led_off durumuna geçerken ilk yapýlacaklar.baþlangýçta yapýlacak iþler çok fazlaysa, nefes aldýrmak için daha faydalý.
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk geçiþ saati, t1 güncel saat
  
  t1 = clock(); // bu fonksiyona girdiðinde o andaki saat
  
  switch (state){
  case I_LED_OFF:
    t0 = t1;
    IO_Write(IOP_LED, 0);
    state = S_LED_OFF;
    //break;
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiþ demek
      state = I_LED_ON;
    }
    break;
    
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 1);
    state = S_LED_ON;
    //break;
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiþ demek
      state = I_LED_OFF;
    }    
    break;
  }
}

void Task_Print(void)
{
  static unsigned count;
  
  //printf("SAYI:%u\r", ++count);
  UART_printf("Count: %10lu\r", ++count);
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();
  
  //printf("\n\nHello World\n"); // dizi yerine bu fonksiyonu kullandýðýmýzda printf'in özelliklerinden mahrum kalmýþ olur.format specifier gibi þeyleri kullanamzdýk.
  UART_puts("\n\nMerhaba dunya!\n");

  /* 05.08.2021
  printf("\rBirinci SATIR");
  printf("\nikinci SATIR");
  printf("\f"); // temizle
  printf("\r1. SATIR");
  printf("\n2. SATIR");
  */
  
  /* 03.08.2021
  LCD_DisplayOn(LCD_MODE_ON | LCD_MODE_CURSOR);
  
  LCD_SetCursor(0x04);
  LCD_PutChar('D'); // cursor otomatik 1 arttýðýndan dolayý
  LCD_PutChar('e');
  LCD_PutChar('n');
  LCD_PutChar('e');
  LCD_PutChar('m');
  LCD_PutChar('e');
  
  LCD_SetCursor(0x40); // 64: ikinci satýr baþý
  LCD_PutChar('T'); // cursor otomatik 1 arttýðýndan dolayý
  LCD_PutChar('e');
  LCD_PutChar('s');
  LCD_PutChar('t');
  */
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev)
  while (1)
  {
    Task_LED(); //diðer tasklarýn doðru çalýþýp çalýþmadýðýný,aksama olup olmadýðýný anlamak için kullanýlabilir.Akýþ devam ediyorsa LED düzgün yanýp sönüyordur.
    //Task_Print();

    // Echo test
    {
      unsigned char c;
      if(UART_DataReady(g_conUART)){
	c = UART_Recv(g_conUART);
	UART_Send(g_conUART, c);
      }
    }

    /*{
	unsigned char c;
	if(UART_DataReady(g_conUART)){
		UART_putch('\n');
		do {
			c = UART_Recv(g_conUART);
			UART_Send(g_conUART, c);
		} while (c != '\r');
		UART_putch('\n');
	}
    }*/
  }
  //return 0;
}


