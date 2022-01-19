#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
#include "timer.h"

#define SYS_CLOCK_FREQ  72000000

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console baþlangýç
  Sys_ConsoleInit();
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
  OLED_SetCursor(1, 0);
  printf("T1Count:%5u\n", g_T1Count);
  printf("T2Count:%5u\n", g_T2Count);
}

void Task_Capture(void)
{
  static unsigned long count;
  
  OLED_SetCursor(1, 0);
  printf("Count:%9lu\n", ++count);
  printf("Capt:%10u\n", g_Capture);
  printf("Freq:%10u", 8 * SYS_CLOCK_FREQ / g_Capture); // IC prescaler DIV8 oldugundan 8 ile carpiliyor.
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  OLED_SetFont(FNT_LARGE);
  printf("Hello, world!\n");

  Capture_Init();
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    //Task_Print();
    Task_Capture();    
  }
  
  //return 0;
}


