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

int g_PWMPeriod;

void Task_ADC(void)
{
  int result, duty;
  //unsigned long freq, freqMin, freqMax;
  float v, t;

  OLED_SetCursor(0, 0);
  
  //////////////////////////////
  result = IADC_Convert(4);    
    
  // Duty cycle pot ile deðiþiyor
  duty = result * g_PWMPeriod / 4095; 
  PWM_Duty(duty);
  
  /*
  // Frekans pot ile deðiþsin
  freqMin = 1000;
  freqMax = 1000000;
  freq = freqMin + (freqMax - freqMin) / 4095 * result;
  PWM_Init(freq, 50);
  */
  
  printf("Ch[4]= %5d\n", result);
  //////////////////////////////
  result = IADC_Convert(17);      
  v = result * 3.3 / 4095;
  printf("Vref=%6.2f\n", v);  
  //////////////////////////////
  result = IADC_Convert(16);    
  v = result * 3.3 / 4095;
  
  t = 25 + (1.43 - v) / 0.0043;
  printf("Temp=%6.2f\n", -t);  
  //////////////////////////////
}

void Task_ADC_Int(void)
{
  float v, t;
  int result;
  
  if (g_bEOC) {
    result = IADC_Result();    
    v = result * 3.3 / 4095;
    
    t = 25 + (1.43 - v) / 0.0043;
    OLED_SetCursor(0, 0);
    printf("Temp=%6.2f", -t);  
   
    // Bir sonraki dönüþümü baþlatýyoruz
    // Eðer sürekli modda isek gerek olmayacak
    //IADC_Start();       
    g_bEOC = 0;
  }
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  OLED_SetFont(FNT_LARGE);
  //printf("Hello, world!\n");

  /*Timer_Init(TIMER_1, SYS_CLOCK_FREQ / 2000, 2000, 1);
  Timer_IntConfig(TIMER_1, 3);
  Timer_Start(TIMER_1, 1);*/
     
  /*
  Timer_Init(TIMER_2, SYS_CLOCK_FREQ / 2000, 2000, 1);
  Timer_IntConfig(TIMER_2, 3);
  Timer_Start(TIMER_2, 1);
  */

  g_PWMPeriod = PWM_Init(10000, 50);
  
  //////////////////////////////////////////
  
  // Ya surekli cevrim yapmak icin IADC_Start cagirilacak ya da continious mod acik olacak.(IADC_Init fonksiyonunda).
  // Surekli IADC_Start cagrilmazsa 1 kere cevrim yapar birakir.
  IADC_IoInit(IOP_AIN4);
  //IADC_Init(1, 0, 0);
  IADC_Init(1, 1, 0); // surekli mod
  
  IADC_IntConfig();
  IADC_Channel(16, 1); // Kanalý listeye ekliyoruz (rank 1)
  IADC_Start(); // Conversion software start.continious modda oldugumuzdan 1 kere start yapmamiz yeterli.
  //////////////////////////////////////////
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    
    //Task_ADC();
    Task_ADC_Int();
  }
  
  //return 0;
}


