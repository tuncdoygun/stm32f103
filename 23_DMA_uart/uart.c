#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "system.h"
#include "io.h"
#include "uart.h"

#include "stm32f10x_usart.h"

int g_conUART = UART_1; // uart-1 konsol olarak kullanýlýyor.

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

  // 1 - I/O Uçlarý yapýlandýrýlýr.
  // TX ucu yapýlandýrmasý
  IO_Init(_uCfg[idx].ioTx, IO_MODE_ALTERNATE);

  // RX ucu yapýlandýrmasý
  IO_Init(_uCfg[idx].ioRx, IO_MODE_INPUT);

  // 2 - UART çevresel birim için clock saðlanýr.
  if (idx == UART_1)
    RCC_APB2PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);
  else
    RCC_APB1PeriphClockCmd(_uCfg[idx].ckUART, ENABLE);

  // 3 - Init yapýsý baþlatýlýr.
  uInit.USART_BaudRate = baud;
  uInit.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  uInit.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
  uInit.USART_Parity = USART_Parity_No;
  uInit.USART_StopBits = USART_StopBits_1;
  uInit.USART_WordLength = USART_WordLength_8b;
  USART_Init(_uCfg[idx].pUSART, &uInit);

  // 4 - Çevresel aktif edilir.
  USART_Cmd(_uCfg[idx].pUSART, ENABLE);
}

void UART_Send(int idx, unsigned char val)
{
  // Strateji 1 - TSR yükleme için uygun mu(boþ mu)
  // TSR Dolu olduðu müddetçe bekle
  while(!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TXE));

  // Yüklemeyi yap
  USART_SendData(_uCfg[idx].pUSART, val);
}

void UART_Send2(int idx, unsigned char val)
{
  // 1 - Yüklemeyi yap
  USART_SendData(_uCfg[idx].pUSART, val);

  // 2 - Veri gidene dek bekle
  while(!USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_TC));
}

// true ise RDR'den veri çekilebilecek
int UART_DataReady(int idx)
{	// RXNE : veri alma registerý boþ deðil, yani veri var.veri hazýr.
  return USART_GetFlagStatus(_uCfg[idx].pUSART, USART_FLAG_RXNE);
}

// Bloke çalýþýr, hazýr veri yoksa bekler.
unsigned char UART_Recv(int idx)
{
  while(!UART_DataReady(idx));

  return (unsigned char)USART_ReceiveData(_uCfg[idx].pUSART);
}

void UART_putch(unsigned char c)
{
  if (c == '\n')
    UART_Send2(g_conUART, '\r'); // \n ise baþýna \r ekle.Böyle olmazsa çapraz kayarak akar.

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

////////////////////////////////////////////////////////////////////////////////
// UART_1 DMA

volatile int g_bUart1Rx, g_bUart1Tx = 1;

void UART1_InitTxDMA(void)
{
  // 1) DMA clock aktif olmalý
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  // 2) UART TXDR Empty olayý ile DMA tetikleme
  USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
  
  // 3) NVIC DMA kesme ayarlarý
  NVIC_SetPriority(DMA1_Channel4_IRQn, 2); // kesme istenilen miktarda veri gonderildiginde olusacak.
  NVIC_EnableIRQ(DMA1_Channel4_IRQn);
}

void UART1_InitRxDMA(void)
{
  // 1) DMA clock aktif olmalý
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  
  // 2) UART RXDR dolu olayý ile DMA tetikleme
  USART_DMACmd(USART1, USART_DMAReq_Rx, ENABLE);
  
  // 3) NVIC DMA kesme ayarlarý
  NVIC_SetPriority(DMA1_Channel5_IRQn, 2);
  NVIC_EnableIRQ(DMA1_Channel5_IRQn);
}

// DMA aracýlýðýyla UART1'den veri gönderir
// 10 kbyte'lik bir veriyi gondermek icin sadece setup yapacak.
// yani gondermesini beklemeyecek.Bir kere ayar yapip ardindan araya CPU girmeden kendiliginden veri gonderilecek.
void UART1_SendDMA(const void *buf, int len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Channel4); // DMA'yý sýfýrla (ilgili kanal),onceden kullanimdaysa.
  
  // DMA kesmesi çevresel taraf ayarlarý
  DMA_ITConfig(DMA1_Channel4, DMA_IT_TC, ENABLE); // UART'in degil,DMA'in kesmesi.
  DMA_ClearITPendingBit(DMA1_IT_TC4);
  
  DMA_StructInit(&dmaInit);     // Gerek olmayabilir,default degerleri ile baslatir.
  
  // Kaynak(memory) -> Destination(UART - periph)
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len; // Transfer blok sayýsý
  dmaInit.DMA_DIR = DMA_DIR_PeripheralDST;
  dmaInit.DMA_M2M = DISABLE;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Normal;
    
  // Kaynak (source) ayarlarý
  dmaInit.DMA_MemoryBaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
      
  // Hedef (destination) ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DISABLE;
  
  DMA_Init(DMA1_Channel4, &dmaInit);
  
  g_bUart1Tx = 0;
  
  // Transferi baþlatýyoruz
  DMA_Cmd(DMA1_Channel4, ENABLE);
}                   
                   
// DMA aracýlýðýyla UART1'den veri alýr
// Circular buffer kullanacaðýz
// bir kere baslatmak yeterli,daha sonra kendiliginden almaya baslicak.
void UART1_RecvDMA(void *buf, int len)
{
  DMA_InitTypeDef dmaInit;
  
  DMA_DeInit(DMA1_Channel5); // DMA'yý sýfýrla (ilgili kanal)
  
  // DMA kesmesi çevresel taraf ayarlarý
  DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, ENABLE);
  DMA_ClearITPendingBit(DMA1_IT_TC5);
  
  DMA_StructInit(&dmaInit);     // Gerek olmayabilir
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = len; // Transfer blok sayýsý
  dmaInit.DMA_DIR = DMA_DIR_PeripheralSRC;
  dmaInit.DMA_M2M = DISABLE;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular;
    
  // Kaynak (source) ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  dmaInit.DMA_PeripheralInc = DISABLE;

  // Hedef (destination) ayarlarý
  dmaInit.DMA_MemoryBaseAddr = (uint32_t)buf;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;
        
  DMA_Init(DMA1_Channel5, &dmaInit);
  
  // Transferi baþlatýyoruz
  DMA_Cmd(DMA1_Channel5, ENABLE);
}                   
                   
////////////////////////////////////////////////////////////////////////////////

void DMA1_Channel4_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC4) == SET) {
    g_bUart1Tx = 1;
    
    DMA_ClearITPendingBit(DMA1_IT_TC4);
  }
}

void DMA1_Channel5_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC5) == SET) {
    g_bUart1Rx = 1;
    
    DMA_ClearITPendingBit(DMA1_IT_TC5);
  }
}
