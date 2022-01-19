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
  //printf("T2Count:%5u\n", g_T2Count);
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  OLED_SetFont(FNT_LARGE);
  printf("Hello, world!\n");

  Timer_Init(TIMER_1, SYS_CLOCK_FREQ / 2000, 2000, 5); // prescale 36000.CNT'ye 0,5 ms'de bir pulse gidecek.autoreload 2000:yani 0,5 ms'yi 2000 kere sayip repetition counter'i tetikleyecek. 0,5ms*2000 = 1 sn
  Timer_IntConfig(TIMER_1, 3);                         // repetition counter 5, 1sn * 5 = 5sn'de kesme.
  Timer_Start(TIMER_1, 1);
  /*           
  Timer_Init(TIMER_2, SYS_CLOCK_FREQ / 2000, 2000, 1); // son parametre(repetition counter) timer 2 icin gecerli degil.
  Timer_IntConfig(TIMER_2, 3);
  Timer_Start(TIMER_2, 1);
  */

  PWM_Init(9000000, 20);  
  DelayMs(700);
  PWM_Init(9000000, 30);  
  DelayMs(700);
  PWM_Init(9000000, 40);  
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    Task_Print();
  }
  
  //return 0;
}


