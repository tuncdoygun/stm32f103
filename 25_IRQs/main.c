#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"
#include "oled.h"
#include "timer.h"
#include "button.h"
#include "exti.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portları başlangıç
  Sys_IoInit();
  
  // LED başlangıç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console başlangıç
  Sys_ConsoleInit();

  // Button başlangıç
  BTN_InitButtons();
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk ge�i� saati
  clock_t t1;           // G�ncel saat de�eri
  
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

// Test amaçlý görev fonksiyonu
void Task_Print(void)
{
  OLED_SetCursor(0, 0);
  printf("%8lu\n", g_T3Count);
  printf("%8lu", g_T4Count);
}

void Task_Button(void)
{
  static unsigned long count;
  
  if (g_Buttons[0]) {
    OLED_SetCursor(0, 0);
    printf("B0:%5lu", ++count);    
    
    g_Buttons[0] = 0;
  }
}

/**
@code  
 The table below gives the allowed values of the pre-emption priority and subpriority according
 to the Priority Grouping configuration performed by NVIC_PriorityGroupConfig function
  ============================================================================================================================
    NVIC_PriorityGroup   | NVIC_IRQChannelPreemptionPriority | NVIC_IRQChannelSubPriority  | Description
  ============================================================================================================================
   NVIC_PriorityGroup_0  |                0                  |            0-15             |   0 bits for pre-emption priority
                         |                                   |                             |   4 bits for subpriority
  ----------------------------------------------------------------------------------------------------------------------------
   NVIC_PriorityGroup_1  |                0-1                |            0-7              |   1 bits for pre-emption priority
                         |                                   |                             |   3 bits for subpriority
  ----------------------------------------------------------------------------------------------------------------------------    
   NVIC_PriorityGroup_2  |                0-3                |            0-3              |   2 bits for pre-emption priority
                         |                                   |                             |   2 bits for subpriority
  ----------------------------------------------------------------------------------------------------------------------------    
   NVIC_PriorityGroup_3  |                0-7                |            0-1              |   3 bits for pre-emption priority
                         |                                   |                             |   1 bits for subpriority
  ----------------------------------------------------------------------------------------------------------------------------    
   NVIC_PriorityGroup_4  |                0-15               |            0                |   4 bits for pre-emption priority
                         |                                   |                             |   0 bits for subpriority                       
  ============================================================================================================================
@endcode
*/

int _addint(int v1, int v2);

int main()
{
  init();
  
  OLED_SetFont(FNT_BIG);
  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  EXTI_IntConfig(IOP_BTN1, I_FALLING, 7, 1);
  
  NVIC_SetPriority(EXTI4_IRQn, 8);
  NVIC_EnableIRQ(EXTI4_IRQn);
  
  Timer_IntConfig(TIMER_3, 11);
  
  Timer_IntConfig(TIMER_4, 10);
  
  /*
  //NVIC_SetPendingIRQ(EXTI4_IRQn);
  NVIC->STIR = EXTI4_IRQn;
  */
  
  /*
  //int is;  
  //is = __ints(0);
  
  __delay(83);
  
  int c;
  c = _addint(17, 24);
  printf("c=%d", c);
  */
  
  while (1)
  {
    Task_LED();
    Task_Print();
  }
}

