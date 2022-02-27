#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "uart.h"
#include "fifo.h"

#define SZ_BUF  256

#define IUART_ST        USART1
#define IUART           UART_1
#define IUART_IRQn      USART1_IRQn

static unsigned char _RxBuf[SZ_BUF];
static unsigned char _TxBuf[SZ_BUF];

static FIFO     _RxFifo, _TxFifo;

void IUART_Init(void)
{
  // IUART baþlangýç
  UART_Init(IUART, 9600);
  
  // FIFO baþlangýç
  FIFO_Init(&_RxFifo, _RxBuf, SZ_BUF);
  FIFO_Init(&_TxFifo, _TxBuf, SZ_BUF);
  
  // Interrupt ayarlarý
  // a) ST yakasý
  USART_ITConfig(IUART_ST, USART_IT_RXNE, ENABLE);
  //USART_ITConfig(IUART_ST, USART_IT_TXE, ENABLE); // Kapalý olmalý,cunku acik olursa islemci acilir acilmaz kesmeye girer.
  
  // b) ARM takasý
  NVIC_SetPriority(IUART_IRQn, 3);
  NVIC_EnableIRQ(IUART_IRQn);
}

volatile static int _txFlag; // mevcut gonderme islemi var mi yok mu

void USART1_IRQHandler(void)
{
  unsigned char c;
  
  if (USART_GetITStatus(IUART_ST, USART_IT_RXNE)) {
    // gelen veriyi alýyoruz
    c = USART_ReceiveData(IUART_ST);
    
    // FIFO buffer'a ekliyoruz
    FIFO_SetData(&_RxFifo, c);
  }
  
  if (USART_GetITStatus(IUART_ST, USART_IT_TXE)) {
    // Tx FIFO'dan veri çekiyoruz
    c = FIFO_GetData(&_TxFifo);
    
    // Veriyi TDR'a yüklüyoruz
    USART_SendData(IUART_ST, c);
    
    // Gönderilecek veri kalmadýysa TX kesme kaynaðý kapatýlmalý
    if (FIFO_IsEmpty(&_TxFifo)) {
      USART_ITConfig(IUART_ST, USART_IT_TXE, DISABLE); 
      _txFlag = 0;
    }
  }  
}

void IUART_SendData(const void *buf, int len)
{
  const unsigned char *ptr = (const unsigned char *)buf;
  
  // Önce Tx FIFO'ya göndereceðimiz veriyi yüklüyoruz
  while (!FIFO_IsFull(&_TxFifo) && (len-- > 0))
    FIFO_SetData(&_TxFifo, *ptr++);
  
  _txFlag = 1;
  // Gönderme kesmesini açýyoruz
  USART_ITConfig(IUART_ST, USART_IT_TXE, ENABLE); 
}

void Task_IUART(void)
{
  char str[256];
  int val;
  unsigned char lineBuf[256];
  int len;
  static unsigned long count;
  
  if (FIFO_GetLine(&_RxFifo, lineBuf)) {
    printf("%s", lineBuf);
  }
  
  /*
  if(val = FIFO_GetData(&_RxFifo)){
    OLED_SetCursor(2, 0); 
    printf("%c", val);
  }
  */
    
  
  if (!_txFlag) { // mevcut gonderme islemi yoksa
    len = sprintf(str, "Bu deneme kesmeli amacli gonderme islemi: %lu\r\n",
            ++count);
    IUART_SendData(str, len);
  }
}

