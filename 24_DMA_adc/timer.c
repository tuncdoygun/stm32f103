#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "timer.h"

TIM_TypeDef     *TimTab[] = {
  TIM1, TIM2, TIM3, TIM4
};

uint32_t TimRccTab[] = {
  RCC_APB2Periph_TIM1,
  RCC_APB1Periph_TIM2,
  RCC_APB1Periph_TIM3,
  RCC_APB1Periph_TIM4,
};

IRQn_Type TimIrqTab[] = {
  TIM1_UP_IRQn,
  TIM2_IRQn,
  TIM3_IRQn,
  TIM4_IRQn,
};

void Timer_Init(int tmNo, unsigned prescale, unsigned period,
                unsigned repeat)
{
  TIM_TimeBaseInitTypeDef tmInit;
  
  // 1) Çevresel clock saðlýyoruz
  if (tmNo == TIMER_1)
    RCC_APB2PeriphClockCmd(TimRccTab[tmNo], ENABLE);
  else
    RCC_APB1PeriphClockCmd(TimRccTab[tmNo], ENABLE);
    
  // 2) Timer parametrelerini ayarlýyoruz
  tmInit.TIM_ClockDivision = TIM_CKD_DIV1;
  tmInit.TIM_CounterMode = TIM_CounterMode_Up;
  tmInit.TIM_Period = period - 1;
  tmInit.TIM_Prescaler = prescale - 1;
  tmInit.TIM_RepetitionCounter = repeat - 1;
  
  TIM_TimeBaseInit(TimTab[tmNo], &tmInit);
  
  Timer_Reset(tmNo);
}

void Timer_Start(int tmNo, int bStart)
{
  TIM_Cmd(TimTab[tmNo], bStart ? ENABLE : DISABLE);
}

void Timer_Reset(int tmNo)
{
  TIM_SetCounter(TimTab[tmNo], 0);
}

void Timer_IntConfig(int tmNo, int priority)
{
  // 1) Çevresel birim ayarlarý
  // a) False interrupt önlemi
  TIM_ClearITPendingBit(TimTab[tmNo], TIM_IT_Update);
  // b) Çevresel yakada kesme kaynaðýný aktive ediyoruz
  TIM_ITConfig(TimTab[tmNo], TIM_IT_Update, ENABLE);
  
  // 2) NVIC (çekirdek) ayarlarý
  // a) Öncelik ayarý
  NVIC_SetPriority(TimIrqTab[tmNo], priority);
  // b) interrupt enable
  NVIC_EnableIRQ(TimIrqTab[tmNo]);
}

////////////////////////////////////////////////////////////////////////////////

// TIM4 CH4 kullanýlacak
// period: ms olarak
void PWM_InitT4CC4(int period)
{

  // 2) Timer ayarlarý
  Timer_Init(TIMER_4, 36000, 2 * period, 1);
  
  // 3) PWM ayarlarý
  // Output compare
  TIM_OCInitTypeDef ocInit;
  
  ocInit.TIM_OCIdleState = TIM_OCIdleState_Reset;
  ocInit.TIM_OCMode = TIM_OCMode_PWM1;
  ocInit.TIM_OCPolarity = TIM_OCPolarity_High;
  ocInit.TIM_OutputState = TIM_OutputState_Enable;
  ocInit.TIM_Pulse = period;
  
  TIM_OC4Init(TIM4, &ocInit);
  
  // 4) Timer'ý çalýþtýrýyoruz
  Timer_Start(TIMER_4, 1);
  
  // 5) TIM4 CC4 Output Enable
  TIM_CtrlPWMOutputs(TIM4, ENABLE);
}

void PWM_Duty(int duty)
{
  TIM_SetCompare3(TIM2, duty);
}

////////////////////////////////////////////////////////////////////////////////

volatile unsigned long g_T1Count, g_T2Count;

void TIM1_UP_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
    ++g_T1Count;
    
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  }
}

void TIM2_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET) {
    ++g_T2Count;
    
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}

void TIM3_IRQHandler(void)
{
}

void TIM4_IRQHandler(void)
{
}



