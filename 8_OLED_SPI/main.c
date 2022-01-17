#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1); // 0 olarak baþlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console baþlangýç
  Sys_ConsoleInit();  
}

// 29.07.2021
// yukardakine göre daha optimize
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
//

void DrawSin(void)
{
  int x, y;
  float fx, pi = 3.14159265359;
  
  // y = A * sin(2 * pi * x);
  for (x = 0; x < 128; ++x) {
    fx = 32 + 30.0 * sin(2 * pi * (x / 50.0));
    y = 63 - (int)(fx + 0.5);
    OLED_SetPixel(x, y, OLED_SETPIXEL);
  }
}

void Task_Print(void)
{
  static unsigned count;
  
  printf("SAYI:%10u\r", ++count);
}

void OLED_Test(void)
{
  int c;
  
  OLED_Start(1);
  
  OLED_SetPixel(0, 0, OLED_SETPIXEL);
  OLED_SetPixel(0, 5, OLED_SETPIXEL);
  OLED_SetPixel(127, 63, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_SETPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(61, 19, OLED_INVPIXEL);
  OLED_SetPixel(0, 5, OLED_CLRPIXEL);

  OLED_SetPage(2);
  OLED_SetSegment(26);
  
  OLED_Data(0x7E);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x11);
  OLED_Data(0x7E);
  
  c = OLED_GetPixel(0, 0);
  c = OLED_GetPixel(0, 1);
  
  OLED_Line(37, 11, 93, 55, OLED_SETPIXEL);
  OLED_Line(127, 0, 0, 63, OLED_SETPIXEL);
  
  OLED_Circle(61, 30, 15, OLED_SETPIXEL);
  OLED_Circle(41, 17, 6, OLED_SETPIXEL);
  
  DrawSin();
  
  OLED_PixelData(5, 40, OLED_SETPIXEL);
  OLED_PixelData(6, 39, OLED_SETPIXEL);
  OLED_PixelData(7, 38, OLED_SETPIXEL);
  OLED_PixelData(8, 37, OLED_SETPIXEL);
  OLED_UpdateDisplay();

  /*
  for (c = 0; c < NPGS * NSEG; ++c)
    _DspRam[c] = 0xAA;
  
  OLED_UpdateDisplay();  
  */
  
  OLED_Scroll(1);
  OLED_Scroll(2);
  OLED_Scroll(1);

  OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(3, 13);
  OLED_PutChar('D');
  OLED_PutChar('e');
  OLED_PutChar('n');
  OLED_PutChar('e');
  OLED_PutChar('m');
  OLED_PutChar('e');
}  
  

int main()
{
  
  // Baþlangýç yapýlandýrmalarý
  init();

  /*
  OLED_SetFont(FNT_LARGE);
  printf("Hello, world!\n");
  OLED_SetFont(FNT_SMALL);
  OLED_SetCursor(2, 0);
  printf("Merhaba dünya!\n");
  */
  
  //OLED_Test();
  
  //DrawSin();
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev)
  while (1)
  {
    Task_LED(); //diðer tasklarýn doðru çalýþýp çalýþmadýðýný,aksama olup olmadýðýný anlamak için kullanýlabilir.Akýþ devam ediyorsa LED düzgün yanýp sönüyordur.
    Task_Print();  
}
  //return 0;
}


