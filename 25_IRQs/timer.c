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
  
  // 1) �evresel clock sa�l�yoruz
  if (tmNo == TIMER_1)
    RCC_APB2PeriphClockCmd(TimRccTab[tmNo], ENABLE);
  else
    RCC_APB1PeriphClockCmd(TimRccTab[tmNo], ENABLE);
    
  // 2) Timer parametrelerini ayarl�yoruz
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

void Timer_EvtClear(int tmNo)
{
    TIM_ClearITPendingBit(TimTab[tmNo], TIM_IT_Update);
    NVIC_ClearPendingIRQ(TimIrqTab[tmNo]);
}

void Timer_EvtConfig(int tmNo)
{
  // 1) �evresel birim ayarlar�
  // a) False interrupt �nlemi
  TIM_ClearITPendingBit(TimTab[tmNo], TIM_IT_Update);
  // b) �evresel yakada kesme kayna��n� aktive ediyoruz
  TIM_ITConfig(TimTab[tmNo], TIM_IT_Update, ENABLE);
}

void Timer_IntConfig(int tmNo, int priority)
{
  // 1) Çevresel birim ayarları (software int için gerekli değil!)
  // a) False interrupt �nlemi
  TIM_ClearITPendingBit(TimTab[tmNo], TIM_IT_Update);
  // b) Çevresel yakada kesme kaynağını aktive ediyoruz
  TIM_ITConfig(TimTab[tmNo], TIM_IT_Update, ENABLE);
  
  // 2) NVIC (çekirdek) ayarları
  // a) Öncelik ayarı
  NVIC_SetPriority(TimIrqTab[tmNo], priority);
  // b) interrupt enable
  NVIC_EnableIRQ(TimIrqTab[tmNo]);
  
  /*
  NVIC_InitTypeDef nvInit;
  
  nvInit.NVIC_IRQChannel = TimIrqTab[tmNo];
  nvInit.NVIC_IRQChannelPreemptionPriority = 1;
  nvInit.NVIC_IRQChannelSubPriority = 0;
  nvInit.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&nvInit);
  */
}

////////////////////////////////////////////////////////////////////////////////

volatile unsigned long g_T1Count, g_T2Count, g_T3Count, g_T4Count;

void TIM1_UP_IRQHandler(void)
{
  if (TIM_GetITStatus(TIM1, TIM_IT_Update) == SET) {
    
    TIM_ClearITPendingBit(TIM1, TIM_IT_Update);
  }

  ++g_T1Count;
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
  ++g_T3Count;
}

void TIM4_IRQHandler(void)
{
  ++g_T4Count;
}
  

