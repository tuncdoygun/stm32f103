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
  OLED_SetCursor(1, 0);
  printf("T1Count:%10u\n", g_T1Count);
  //printf("T2Count:%10u\n", g_T2Count);
}

#define NSAMPLES        25

void Task_Seismo(void)
{
  static enum {
    S_INIT,
    S_WAIT,
    S_WORK
  } state = S_INIT;
  
  const int ch = 4;
  int result;
  static int x, y, y0;
  static unsigned long count;
  static int total, ns;
  
  switch (state) {
  case S_INIT:
    // TIM1/CC1 50 ms kare dalga
    //PWM_Init(10000);    // 1s test
    //PWM_Init(500);      // 50 ms
    PWM_Init(20);      // 2 ms'de bir ADC cevrim yapacak.
    
    // ADC baþlangýç
    IADC_IoInit(IOP_AIN4);
    
    // External trigger kullanýrken sürekli mod
    // devre dýþý olmalý!
    IADC_Init(1, 0, 0);
    
    IADC_Channel(ch, 1);
    IADC_IntConfig();
    
    state = S_WAIT;
    break;
    
  case S_WAIT:
    if (g_bEOC)
      state = S_WORK;
    break;
    
  case S_WORK:
    result = IADC_Result();
    /*
    OLED_SetCursor(0, 0);
    printf("Count:%9lu\nCh[4]:%6u", ++count, result);
    */
        
    total += result;
    
    if (++ns >= NSAMPLES) { // 50ms olan orneklemeyi 2ms e cekip ortalamasini alip 50 ms de yazdirma
      ns = 0;
      
      y = (total / NSAMPLES) / 64;
      
      /*
      if (y > 48) {
        OLED_SetCursor(0, 0);
        printf("%10lu", ++count);
      }
      */
      
      total = 0;
    
      if (x == 0)
        OLED_ClearDisplay();
      else
        OLED_Line(x - 1, y0, x, y, OLED_SETPIXEL);
      
      if (++x == 128) 
        x = 0;
      
      y0 = y;
    }
    
    g_bEOC = 0;
    state = S_WAIT;
    break;
  }
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  OLED_SetFont(FNT_LARGE);
  //printf("Hello, world!\n");

  //////////////////////////////////////////
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    
    Task_Seismo();
  }
  
  //return 0;
}


