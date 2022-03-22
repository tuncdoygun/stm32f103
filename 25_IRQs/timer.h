#ifndef _TIMER_H
#define _TIMER_H

enum {
  TIMER_1,
  TIMER_2,
  TIMER_3,
  TIMER_4,
};


extern volatile unsigned long g_T1Count, g_T2Count, g_T3Count, g_T4Count;

void Timer_Init(int tmNo, unsigned prescale, unsigned period,
                unsigned repeat);
void Timer_Start(int tmNo, int bStart);
void Timer_Reset(int tmNo);
void Timer_IntConfig(int tmNo, int priority);
void Timer_EvtConfig(int tmNo);
void Timer_EvtClear(int tmNo);


#endif

