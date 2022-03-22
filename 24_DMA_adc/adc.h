#ifndef _ADC_H
#define _ADC_H

extern volatile int g_bEOC;    // End of conversion flag

void IADC_IoInit(int idx);
void IADC_Channel(int ch, int rank);
void IADC_Init(int nConv, int cmode, int smode);
void IADC_Start(void);
int IADC_Result(void);
int IADC_Convert(int ch);
void IADC_IntConfig(void);

#endif