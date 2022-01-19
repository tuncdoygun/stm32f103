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

/*
  prescale : 16-bit prescale
  period :   AutoReload'a yazilacak deger.
  repeat :   sadece timer1 icin gecerli.Advanced timer icin.
*/
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
  TIM_ClearITPendingBit(TimTab[tmNo], TIM_IT_Update); // reload tastigi zaman olusur.
  // b) Çevresel yakada kesme kaynaðýný aktive ediyoruz
  TIM_ITConfig(TimTab[tmNo], TIM_IT_Update, ENABLE);
  
  // 2) NVIC (çekirdek) ayarlarý
  // a) Öncelik ayarý
  NVIC_SetPriority(TimIrqTab[tmNo], priority);
  // b) interrupt enable
  NVIC_EnableIRQ(TimIrqTab[tmNo]);
}

////////////////////////////////////////////////////////////////////////////////

void Capture_Init(void)
{
  TIM_ICInitTypeDef icInit;
  
  // 1) I/O ayarý
  IO_Init(IOP_TIM4_CH1, IO_MODE_INPUT);
  
  // 2) Timer baþlangýç
  Timer_Init(TIMER_4, 1, 65536, 1);
  
  // 3) Input Capture baþlangýç
  icInit.TIM_Channel = TIM_Channel_1;
  icInit.TIM_ICFilter = 0;
  icInit.TIM_ICPolarity = TIM_ICPolarity_Falling; // genelde dusen kenar almak daha mantikli.Cunku dusme zamanlari daha hizli.
  icInit.TIM_ICPrescaler = TIM_ICPSC_DIV8;        // kac eventte capture alindin.
  icInit.TIM_ICSelection = TIM_ICSelection_DirectTI;
  
  TIM_ICInit(TIM4, &icInit);
  
  // 3) Interrupt config
  TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
  TIM_ITConfig(TIM4, TIM_IT_CC1, ENABLE);
  NVIC_SetPriority(TIM4_IRQn, 0);
  NVIC_EnableIRQ(TIM4_IRQn);
  
  // 4) Timer'ý çalýþtýrýyoruz
  TIM_Cmd(TIM4, ENABLE); 
}

////////////////////////////////////////////////////////////////////////////////

volatile unsigned long g_T1Count, g_T2Count, g_T3Count, g_T4Count;

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

volatile uint16_t g_Capture;

void TIM4_IRQHandler(void)
{
  static uint16_t c0, c1;
  
  if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET) {
    ++g_T4Count;
    
    TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
  }
  
  if (TIM_GetITStatus(TIM4, TIM_IT_CC1) == SET) {
    c1 = TIM_GetCapture1(TIM4);
    
    g_Capture = c1 - c0;
    c0 = c1;
    
    TIM_ClearITPendingBit(TIM4, TIM_IT_CC1);
  }  
}



