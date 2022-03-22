#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"
#include "oled.h"
#include "exti.h"
#include "ertc.h"

void init(void)
{
  // System Clock init
  //Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console baþlangýç
  Sys_ConsoleInit();

  // Send Event on Pending (Peripheral Event)
  SCB->SCR |= SCB_SCR_SEVONPEND;  
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk geçiþ saati
  clock_t t1;           // Güncel saat deðeri
  
  t1 = clock();
  
  switch (state) {
  case I_LED_OFF:
      t0 = t1;      
      IO_Write(IOP_LED, 1);     // LED off
      state = S_LED_OFF;
      //break;    
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 0);     // LED On
    state = S_LED_ON;
    //break;    
  case S_LED_ON:
    if (t1 >= t0 + CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
  }  
}

void Task_Print(void)
{
  static unsigned count;
  
  printf("%8lu\r", ++count);
}

void Task_ERTC(void)
{
  ERTC_DATE dt;
    
  ERTC_GetDate(&dt, 0, 7);
  
  OLED_SetCursor(0, 0);
  printf("%02X:%02X:%02X\n"
         "%02X/%02X/%02X", 
         dt.hour, dt.min, dt.sec,
         dt.date, dt.mon, dt.year);
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  EXTI_IntConfig(IOP_SQW, I_FALLING, 1, 1);
  
  ERTC_Init();
#if 0
  {
    ERTC_DATE dt;
    
    dt.hour = 0x21;
     dt.min = 0x10;
     dt.sec = 0x00; 
     
     dt.day = 0x07;
     
     dt.date = 0x09;
     dt.mon = 0x12;
    dt.year = 0x21;
  
    ERTC_SetDate(&dt, 0, 7);
  }
#endif
  
  OLED_SetFont(FNT_BIG);
  //printf("Hello!\n");
  
  while (1)
  {
    Task_ERTC();
    
    __WFI();
  }
  
  //return 0;
}


