#ifndef _IRTC_H
#define _IRTC_H

extern volatile int g_Time;
extern volatile int g_RtcAlarm;
extern volatile int g_RtcChanged;

void IRTC_Init(void);
uint32_t IRTC_GetTime(void);
void IRTC_SetTime(uint32_t tmVal);
void IRTC_IntConfig(void);
void IRTC_SetAlarm(uint32_t alrVal);

#endif
