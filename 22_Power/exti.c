#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "system.h"

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
  EXTI_Trigger_Rising_Falling,  
};

// bEnable: eventin aktif olup olmayacagi.Interrupt kullanilmayip sadece event kullanilabilir.
// PA0,PB0,PC0... ayni anda EXTI olarak kullanilamaz.cunku multiplexer var.
void EXTI_IntConfig(int idx, int trigger, int priority, int bEnable)
{
  EXTI_InitTypeDef iEXTI;
  int port, pin, line;
  IRQn_Type IRQn;
  
  port = _ios[idx].port;
  pin  = _ios[idx].pin;
  line = _EXTI_Line[pin];
  IRQn = _EXTI_IRQn[pin];
  
  // 1) AFIO clock aktif olmalý
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  
  // 2) EXTIx hattý ile ilgili port baðlantý kurulacak
  GPIO_EXTILineConfig(port, pin);
  
  // 3) EXTI structure baþlangýç deðerleri
  iEXTI.EXTI_Line = line;
  iEXTI.EXTI_Mode = EXTI_Mode_Interrupt;
  iEXTI.EXTI_Trigger = _Trigger[trigger];
  iEXTI.EXTI_LineCmd = ENABLE;
  
  EXTI_Init(&iEXTI);
  
  //////////////////////////////  
  // EXTI interrupt/event setup
  // ST yakasý
  EXTI_ClearITPendingBit(line);
  
  // ARM yakasý
  NVIC_SetPriority(IRQn, priority);
  
  if (bEnable)
    NVIC_EnableIRQ(IRQn);
  else
    NVIC_DisableIRQ(IRQn);    
}

void EXTI_EvtClear(int idx)
{
  int pin, line;
  IRQn_Type IRQn;
  
  pin  = _ios[idx].pin;
  line = _EXTI_Line[pin];
  IRQn = _EXTI_IRQn[pin];
  
  EXTI_ClearITPendingBit(line);
  NVIC_ClearPendingIRQ(IRQn);
}

void EXTI2_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line2) == SET) // A2 buton
  {
    EXTI_ClearITPendingBit(EXTI_Line2);
  }
}

void EXTI9_5_IRQHandler(void)
{
  if (EXTI_GetITStatus(EXTI_Line5) == SET) 
  {
    EXTI_ClearITPendingBit(EXTI_Line5);
  }

  if (EXTI_GetITStatus(EXTI_Line6) == SET) 
  {
    EXTI_ClearITPendingBit(EXTI_Line6);
  }

  if (EXTI_GetITStatus(EXTI_Line7) == SET) 
  {
    EXTI_ClearITPendingBit(EXTI_Line7);
  }

  if (EXTI_GetITStatus(EXTI_Line8) == SET) 
  {
    EXTI_ClearITPendingBit(EXTI_Line8);
  }

  if (EXTI_GetITStatus(EXTI_Line9) == SET) 
  {
    EXTI_ClearITPendingBit(EXTI_Line9);
  }
}


  