#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
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
  
  // Console ba�lang��
  Sys_ConsoleInit();  
  
  BTN_InitButtons();
}

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
  
  OLED_SetCursor(0, 0);
  printf("SAYI:%10u\r", ++count);
}

void ResetProc(void)
{
  char *msg;
  
  if (RCC_GetFlagStatus(RCC_FLAG_PORRST)) 
    msg = "Power On";
  else if (RCC_GetFlagStatus(RCC_FLAG_SFTRST))
    msg = "Software";
  else if (RCC_GetFlagStatus(RCC_FLAG_IWDGRST))
    msg = "Indp WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_WWDGRST))
    msg = "Wind WDG";
  else if (RCC_GetFlagStatus(RCC_FLAG_LPWRRST))
    msg = "LowPower";
  else if (RCC_GetFlagStatus(RCC_FLAG_PINRST)) // external reset sonra olmali.External disindaki reset bu sebepten gerceklestiyse.
    msg = "External";                          // Diger resetler de external reseti gerceklestiriyor.
  
  OLED_SetCursor(1, 0);
  printf(msg);      
  
  RCC_ClearFlag();      // !!!
}

void IWDG_Init(uint16_t tmr) // tmr reset araligi
{
  // 1) IWDG register'lar�na yazma izni olmal�
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
  
  // 2) IWDG clock prescale ayar� (LSI)
  IWDG_SetPrescaler(IWDG_Prescaler_256);
  
  // 3) Reload (periyot) de�eri
  IWDG_SetReload(tmr);  // 12-bit
  
  // 4) Sayac� y�kl�yoruz
  IWDG_ReloadCounter();
  
  // 5) IWDG birimini aktif hale getiriyoruz
  IWDG_Enable();
}

int main()
{
  // Ba�lang�� yap�land�rmalar�
  init();

  OLED_SetFont(FNT_BIG);
  
  ResetProc();
  
  IWDG_Init(0xFFF);  
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev)
  while (1)
  {
    Task_LED();
    // Task_Print();  
  
    /*
    if (g_Buttons[0]) {
      IWDG_ReloadCounter();
      g_Buttons[0] = 0;
    }
    */    
    
    if(g_Buttons[0]) // semaphore'u sifirlamaya gerek yok,zaten karta reset atilacak
      NVIC_SystemReset();
    
    //IWDG_ReloadCounter(); // while icinde tmr sayacindan fazla tikansaydi islemci reset atard�,
                            // fakat timer reload edildiginden sayac tekrar basliyor,yani 0 degerine dusmuyor.
                            // boylelikle watchdog reset atmamis oluyor.
  }
  //return 0;
}


