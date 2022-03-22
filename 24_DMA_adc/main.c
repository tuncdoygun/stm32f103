#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"
#include "oled.h"
#include "timer.h"
#include "adc.h"

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
  static unsigned long count;
  OLED_SetCursor(0, 0);
  
  printf("Count:%10u\n", ++count);
}
////////////////////////////////////////////////////////////////////////////////
uint8_t g_chList[] = { 6, 7, 16, 17, 6, 6, 17, 16 }; // kanal listesi
#define N_CONV  (sizeof(g_chList) / sizeof(uint8_t))
volatile uint16_t g_rsList[N_CONV]; //  sonuclar buraya aktarilacak.

void Task_ADC(void)
{
  int i, ch, rs;
  float v, t;
  
  if (g_bEOC) {
    for (i = 0; i < N_CONV; ++i) {
      ch = g_chList[i];
      rs = g_rsList[i];
      
      v = rs * 3.3 / 4095;
      
      OLED_SetCursor(i, 0);
      if (ch == 16) {
        t = (1.43 - v) / 0.0043 + 25;
        printf("Ch:%2d %4d %4.1f C", ch, rs, t);
      }      
      else
        printf("Ch:%2d %4d %4.2f V", ch, rs, v);
    }
    
    g_bEOC = 0;
  }
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();
  
  IADC_IoInit(IOP_AIN6);
  IADC_IoInit(IOP_AIN7);

  IADC_Init_DMA(N_CONV, g_chList, g_rsList, 1000);
    
  printf("Hello, world!\n");
      
  //////////////////////////////////////////  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    
    Task_ADC();
  }
  
  //return 0;
}
