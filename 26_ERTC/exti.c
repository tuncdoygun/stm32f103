#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "system.h"

////////////////////////////////////////////////////////////////////////////////

static uint32_t _EXTI_Line[] = {
  EXTI_Line0,
  EXTI_Line1,
  EXTI_Line2,
  EXTI_Line3,
  EXTI_Line4,
  EXTI_Line5,
  EXTI_Line6,
  EXTI_Line7,
  EXTI_Line8,
  EXTI_Line9,
  EXTI_Line10,
  EXTI_Line11,
  EXTI_Line12,
  EXTI_Line13,
  EXTI_Line14,
  EXTI_Line15,
};

static IRQn_Type _EXTI_IRQn[] = {
  EXTI0_IRQn,
  EXTI1_IRQn,
  EXTI2_IRQn,
  EXTI3_IRQn,
  EXTI4_IRQn,

  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  EXTI9_5_IRQn,
  
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
  EXTI15_10_IRQn,
};

static EXTITrigger_TypeDef _Trigger[] = {
  EXTI_Trigger_Rising,
  EXTI_Trigger_Falling,  
  EXTI_Trigger_Rising_Falling
};

// trigger: 0=Rising, 1=Falling, 2=Rising/Falling
void EXTI_IntConfig(int idx, int trigger, int priority, int bEnable)
{
  EXTI_InitTypeDef   EXTI_InitStructure;
  int port, pin, line;
  IRQn_Type IRQn;

  port = _ios[idx].port;
  pin = _ios[idx].pin;
  line = _EXTI_Line[pin];
  IRQn = _EXTI_IRQn[pin];
  
  /* Enable AFIO clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  /* Connect EXTIx Line to pin */
  GPIO_EXTILineConfig(port, pin);
  
  /* Configure EXTI line */
  EXTI_InitStructure.EXTI_Line = line; 
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = _Trigger[trigger]; 
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  // Enable and set EXTIx Interrupt to the lowest priority 
  // Kesme kaynaðý aktive ediliyor
  EXTI_ClearITPendingBit(line);   // False interrupt önlemi!
  
  // Kesme vektörü aktive ediliyor (IRQn)
  NVIC_InitTypeDef   NVIC_InitStructure;
  
  NVIC_InitStructure.NVIC_IRQChannel = IRQn; 
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = priority;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = bEnable ? ENABLE : DISABLE;
  
  NVIC_Init(&NVIC_InitStructure);
}

void EXTI_EvtClear(int idx)
{
  EXTI_ClearITPendingBit(_EXTI_Line[idx]);  
}

////////////////////////////////////////////////////////////////////////////////

void EXTI0_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line0) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line0);  
  }
}

void EXTI1_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line1) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line1);  
  }
}

void EXTI2_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line2) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line2);  
  }
}

void EXTI3_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line3) == SET)
  {
    EXTI_ClearITPendingBit(EXTI_Line3);  
  }
}

void EXTI4_IRQHandler(void)
{ 
  if (EXTI_GetITStatus(EXTI_Line4) == SET)
  {
    IO_Write(IOP_LED, 0);
    DelayMs(10);
    IO_Write(IOP_LED, 1);
    
    EXTI_ClearITPendingBit(EXTI_Line4);  
  }
}
