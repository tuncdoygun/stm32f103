#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
//#include "lcdm.h"
#include "uart.h"
#include "button.h"

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
  //LCD_Init();
  
  // Konsol Ba�lang��
  Sys_ConsoleInit();
  
  // Button ba�lang��
  BTN_InitButtons();
}

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

void Task_Print(void)
{
  static unsigned count;
  
  //printf("SAYI:%u\r", ++count);
  UART_printf("Count: %10lu\r", count);
}

void Task_Button(void)
{
  static unsigned count = 0;
  
  if (g_Buttons[BTN_SET]){ // BTN_SET'ten i�aret geldi
    // SET i�le
    UART_printf("SET (%u)\n", ++count);
    
    // g_Buttons[BTN_SET] = 0; binary semaphore
    --g_Buttons[BTN_SET]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_Buttons[BTN_UP]){ // BTN_UP'ten i�aret geldi
    // SET i�le
    UART_printf("UP (%u)\n", ++count);
    
    //g_Buttons[BTN_UP] = 0;
    --g_Buttons[BTN_UP]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_Buttons[BTN_DN]){ // BTN_UP'ten i�aret geldi
    // DN i�le
    UART_printf("DN (%u)\n", ++count);
    
    //g_Buttons[BTN_DN] = 0;
    --g_Buttons[BTN_DN]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  //////////////////////////////////////
#ifdef BTN_LONG_PRESS
  if (g_ButtonsL[BTN_SET]){
    // SET i�le
    UART_printf("SET_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_SET] = 0; //binary semaphore
    //--g_Buttons[BTN_SET]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_ButtonsL[BTN_UP]){ // BTN_UP'ten i�aret geldi
    // SET i�le
    UART_printf("UP_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_UP] = 0;
    // --g_Buttons[BTN_UP]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_ButtonsL[BTN_DN]){ // BTN_UP'ten i�aret geldi
    // DN i�le
    UART_printf("DN_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_DN] = 0;
    // --g_Buttons[BTN_DN]; // s�f�ra gelene kadar hep i�leyecek
  }
#endif
}

int main()
{
  // Ba�lang�� yap�land�rmalar�
  init();
  
  //printf("\n\nHello World\n"); // dizi yerine bu fonksiyonu kulland���m�zda printf'in �zelliklerinden mahrum kalm�� olur.format specifier gibi �eyleri kullanamzd�k.
  UART_puts("\n\nMerhaba dunya!\n");
  
  /* // Counting semaphore test
  UART_printf("Bekleme..."); 
  DelayMs(10000);// beklemedeyken 1 kere butona bas�lm�� gibi olacak.Counting semaphore laz�m
  UART_printf("bitti\n");
  */
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev)
  while (1)
  {
    Task_LED(); //di�er tasklar�n do�ru �al���p �al��mad���n�,aksama olup olmad���n� anlamak i�in kullan�labilir.Ak�� devam ediyorsa LED d�zg�n yan�p s�n�yordur.
    //Task_Print();
    
    Task_Button();
  }
  //return 0;
}


