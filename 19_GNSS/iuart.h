#ifndef _IUART_H
#define _IUART_H

void IUART_Init(void);
void IUART_SendData(const void *buf, int len);

void Task_IUART(void);


#endif

