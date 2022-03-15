#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
#include "button.h"
#include "uart.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1); // 0 olarak ba�latmak i�in
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console ba�lang��
  Sys_ConsoleInit();

  // Button ba�lang��
  BTN_InitButtons();
}

// 29.07.2021
// yukardakine g�re daha optimize
void Task_LED(void)
{
  static enum {
    I_LED_OFF,// led_off durumuna ge�erken ilk yap�lacaklar.ba�lang��ta yap�lacak i�ler �ok fazlaysa, nefes ald�rmak i�in daha faydal�.
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk ge�i� saati, t1 g�ncel saat
  
  t1 = clock(); // bu fonksiyona girdi�inde o andaki saat
  
  switch (state){
  case I_LED_OFF:
    t0 = t1;
    IO_Write(IOP_LED, 0);
    state = S_LED_OFF;
    //break;
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      state = I_LED_ON;
    }
    break;
    
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 1);
    state = S_LED_ON;
    //break;
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      state = I_LED_OFF;
    }    
    break;
  }
}
//

void Task_Print(void)
{
  static unsigned long count;
  int row, col;
  
  OLED_GetCursor(&row, &col);
  
  OLED_SetCursor(0, 0);
  printf("%8lu", ++count);

  OLED_SetCursor(row, col);
}

void Task_Button(void)
{
  static unsigned long count;
  
  if (g_Buttons[0]) {
    OLED_SetCursor(0, 0);
    printf("B0:%5lu", ++count);    
    
    g_Buttons[0] = 0;
  }
}

void Task_UartTx(void)
{
  static unsigned long count;
  static char buf[256];  
  int len;
  
  if (g_bUart1Tx) {
    len = sprintf(buf, "Bu yazi DMA araciligiyle gonderiliyor: %lu\r\n",
                  ++count);
                  
    UART1_SendDMA(buf, len);
    
    // g_bUart1Tx flag i UART1_SendDMA icinde sifirlaniyor.
  }
}

char g_RxBuf[256];
int g_RxLen = 10;

void Task_UartRx(void)
{
  if (g_bUart1Rx) {
      g_RxBuf[g_RxLen] = '\0';
      printf("%s", g_RxBuf);
      
      g_bUart1Rx = 0;
  }
}

int main()
{
  init();

  //OLED_SetFont(FNT_LARGE);
  OLED_SetCursor(1, 0);

  UART_Init(UART_1, 115200);
  
  UART1_InitTxDMA();
  
  UART1_InitRxDMA();
  UART1_RecvDMA(g_RxBuf, g_RxLen); // 10 karakter aldiktan sonra kesme verecek.
  
  while (1)
  {
    Task_LED();
    Task_Print();
    
    Task_UartTx();
    Task_UartRx();
  }
}
