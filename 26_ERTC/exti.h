#ifndef _EXTI_H
#define _EXTI_H

enum {
  I_RISING,
  I_FALLING,
  I_RISING_FALLING,
};

void EXTI_IntConfig(int idx, int trigger, int priority, int bEnable);
void EXTI_EvtClear(int idx);

#endif