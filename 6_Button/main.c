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
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1); // 0 olarak baþlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  //LCDM Baþlangýç
  //LCD_Init();
  
  // Konsol Baþlangýç
  Sys_ConsoleInit();
  
  // Button baþlangýç
  BTN_InitButtons();
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
  UART_printf("Count: %10lu\r", count);
}

void Task_Button(void)
{
  static unsigned count = 0;
  
  if (g_Buttons[BTN_SET]){ // BTN_SET'ten iþaret geldi
    // SET iþle
    UART_printf("SET (%u)\n", ++count);
    
    // g_Buttons[BTN_SET] = 0; binary semaphore
    --g_Buttons[BTN_SET]; // sýfýra gelene kadar hep iþleyecek
  }
  
  if (g_Buttons[BTN_UP]){ // BTN_UP'ten iþaret geldi
    // SET iþle
    UART_printf("UP (%u)\n", ++count);
    
    //g_Buttons[BTN_UP] = 0;
    --g_Buttons[BTN_UP]; // sýfýra gelene kadar hep iþleyecek
  }
  
  if (g_Buttons[BTN_DN]){ // BTN_UP'ten iþaret geldi
    // DN iþle
    UART_printf("DN (%u)\n", ++count);
    
    //g_Buttons[BTN_DN] = 0;
    --g_Buttons[BTN_DN]; // sýfýra gelene kadar hep iþleyecek
  }
  
  //////////////////////////////////////
#ifdef BTN_LONG_PRESS
  if (g_ButtonsL[BTN_SET]){
    // SET iþle
    UART_printf("SET_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_SET] = 0; //binary semaphore
    //--g_Buttons[BTN_SET]; // sýfýra gelene kadar hep iþleyecek
  }
  
  if (g_ButtonsL[BTN_UP]){ // BTN_UP'ten iþaret geldi
    // SET iþle
    UART_printf("UP_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_UP] = 0;
    // --g_Buttons[BTN_UP]; // sýfýra gelene kadar hep iþleyecek
  }
  
  if (g_ButtonsL[BTN_DN]){ // BTN_UP'ten iþaret geldi
    // DN iþle
    UART_printf("DN_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_DN] = 0;
    // --g_Buttons[BTN_DN]; // sýfýra gelene kadar hep iþleyecek
  }
#endif
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();
  
  //printf("\n\nHello World\n"); // dizi yerine bu fonksiyonu kullandýðýmýzda printf'in özelliklerinden mahrum kalmýþ olur.format specifier gibi þeyleri kullanamzdýk.
  UART_puts("\n\nMerhaba dunya!\n");
  
  /* // Counting semaphore test
  UART_printf("Bekleme..."); 
  DelayMs(10000);// beklemedeyken 1 kere butona basýlmýþ gibi olacak.Counting semaphore lazým
  UART_printf("bitti\n");
  */
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev)
  while (1)
  {
    Task_LED(); //diðer tasklarýn doðru çalýþýp çalýþmadýðýný,aksama olup olmadýðýný anlamak için kullanýlabilir.Akýþ devam ediyorsa LED düzgün yanýp sönüyordur.
    //Task_Print();
    
    Task_Button();
  }
  //return 0;
}


