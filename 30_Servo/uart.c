#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "system.h"
#include "io.h"
#include "uart.h"

#include "stm32f10x_usart.h"

int g_conUART = UART_1; // uart-1 konsol olarak kullan�l�yor.

struct UART_CFG {
  int ioTx;
  int ioRx;

  uint32_t ckUART; // clock Uart
  USART_TypeDef * pUSART;
};

static struct UART_CFG _uCfg[] = {
  { IOP_U1TX, IOP_U1RX, RCC_APB2Periph_USART1, USART1},
//  { IOP_U2TX, IOP_U2RX, RCC_APB1Periph_USART2, USART2},
//  { IOP_U3TX, IOP_U3RX, RCC_APB1Periph_USART3, USART3},
};

/*static USART_TypeDef *_tUART[] = {
		USART_1,
		USART_2,
		USART_3
};*/

void UART_Init(int idx, int baud)
{
  USART_InitTypeDef uInit;

  // 1 - I/O U�lar� yap�land�r�l�r.
  // TX ucu yap�land�rmas�
  IO_Init(_uCfg[idx].ioTx, IO_MODE_ALTERNATE);

  // RX ucu yap�land�rmas�
  IO_Init(_uCfg[idx].ioRx, IO_MODE_INPUT);

  // 2 - UART �evresel birim i�in clock sa�lan�r.
  if (idx == UART_1)
    RCC_APB2PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);
  else
    RCC_APB1PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);

  // 3 - Init yap�s� ba�lat�l�r.
  uInit.USART_BaudRate = baud;
  uInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  uInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  uInit.USART_Parity = USART_Parity_No;
  uInit.USART_StopBits = USART_StopBits_1;
  uInit.USART_WordLength = USART_WordLength_8b;
  USART_Init(_uCfg[idx].pUSART, &uInit);

  // 4 - �evresel aktif edilir.
  USART_Cmd(_uCfg[idx].pUSART, ENABLE);
}

void UART_Send(int idx, unsigned char val)
{
  // Strateji 1 - TSR y�kleme i�in uygun mu(bo� mu)
  // TSR Dolu oldu�u m�ddet�e bekle
  while(!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TXE));

  // Y�klemeyi yap
  USART_SendData(_uCfg[idx].pUSART, val);
}

void UART_Send2(int idx, unsigned char val)
{
  // 1 - Y�klemeyi yap
  USART_SendData(_uCfg[idx].pUSART, val);

  // 2 - Veri gidene dek bekle
  while(!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TC));
}

// true ise RDR'den veri �ekilebilecek
int UART_DataReady(int idx)
{	// RXNE : veri alma register� bo� de�il, yani veri var.veri haz�r.
  return USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_RXNE);
}

// Bloke �al���r, haz�r veri yoksa bekler.
unsigned char UART_Recv(int idx)
{
  while(!UART_DataReady(idx));

  return (unsigned char)USART_ReceiveData(_uCfg[idx].pUSART);
}

void UART_putch(unsigned char c)
{
  if (c == '\n')
    UART_Send2(g_conUART, '\r'); // \n ise ba��na \r ekle.B�yle olmazsa �apraz kayarak akar.

  UART_Send2(g_conUART, c);
}

int UART_puts(const char *str)
{
  int i = 0;

  while(str[i])
    UART_putch(str[i++]);

  return i;
}

int UART_printf(const char *fmt, ...)
{
  va_list args;
  char str[SZ_PRNBUF];

  va_start(args, fmt);
  vsnprintf(str, SZ_PRNBUF, fmt, args);

  return UART_puts(str);
}
