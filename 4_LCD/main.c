#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "lcdm.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1); // 0 olarak ba�latmak i�in
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  //LCDM Ba�lang��
  // LCD_Init();
  
  // Konsol Ba�lang��
  Sys_ConsoleInit();
}

//int c; // debug modda global de�i�kenlerin de�eri g�r�lebiliyor.IO_Read() i�in.
// 27.07.2021
/*void Task_LED(void)
{
  static enum {
    S_LED_OFF,
    S_LED_ON,
  } state = S_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk ge�i� saati, t1 g�ncel saat
  
  t1 = clock(); // bu fonksiyona girdi�inde o andaki saat
  
  switch (state){
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      IO_Write(IOP_LED, 0);
      
      t0 = t1;
      state = S_LED_ON;
    }
    break;
  
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      IO_Write(IOP_LED, 1);
      
      t0 = t1;
      state = S_LED_OFF;
    }    
    break;
  }
}*/
//

// 29.07.2021
// yukardakine g�re daha optimize
void Task_LED(void)
{
  static enum {
    I_LED_OFF,// led_off durumuna ge�erken ilk yap�lacaklar.ba�lang��ta yap�lacak i�ler �ok fazlaysa, nefes ald�rmak i�in daha faydal�.
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk ge�i� saati, t1 g�ncel saat
  
  t1 = clock(); // bu fonksiyona girdi�inde o andaki saat
  
  switch (state){
  case I_LED_OFF:
    t0 = t1;
    IO_Write(IOP_LED, 0);
    state = S_LED_OFF;
    //break;
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      state = I_LED_ON;
    }
    break;
    
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 1);
    state = S_LED_ON;
    //break;
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      state = I_LED_OFF;
    }    
    break;
  }
}
//

void Task_Print(void)
{
  static unsigned count;
  
  printf("\nSAYI:%10u", ++count);
}

int main()
{
  // Ba�lang�� yap�land�rmalar�
  init();
  
  printf("Hello World\n"); // dizi yerine bu fonksiyonu kulland���m�zda printf'in �zelliklerinden mahrum kalm�� olur.format specifier gibi �eyleri kullanamzd�k.
 
  //05.08.2021
  printf("\rBirinci SATIR");
  printf("\nikinci SATIR");
  printf("\f"); // temizle
  printf("\r1. SATIR");
  printf("\n2. SATIR");
  
  
  /* 03.08.2021
  LCD_DisplayOn(LCD_MODE_ON | LCD_MODE_CURSOR);
  
  LCD_SetCursor(0x04);
  LCD_PutChar('D'); // cursor otomatik 1 artt���ndan dolay�
  LCD_PutChar('e');
  LCD_PutChar('n');
  LCD_PutChar('e');
  LCD_PutChar('m');
  LCD_PutChar('e');
  
  LCD_SetCursor(0x40); // 64: ikinci sat�r ba��
  LCD_PutChar('T'); // cursor otomatik 1 artt���ndan dolay�
  LCD_PutChar('e');
  LCD_PutChar('s');
  LCD_PutChar('t');
  */
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev)
  while (1)
  {
    Task_LED(); //di�er tasklar�n do�ru �al���p �al��mad���n�,aksama olup olmad���n� anlamak i�in kullan�labilir.Ak�� devam ediyorsa LED d�zg�n yan�p s�n�yordur.
    Task_Print();
  }
  //return 0;
}


