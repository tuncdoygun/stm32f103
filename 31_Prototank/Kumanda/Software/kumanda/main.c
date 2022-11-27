#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "adc.h"
#include "nRF24.h"

#define SYS_CLOCK_FREQ  72000000
#define JOY_CH_Y        0
#define JOY_CH_X        1

#define NSAMPLES        200

enum {
  NONE,
  GREEN_1,
  GREEN_2,
  YELLOW,
  RED
};

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console(oled) baþlangýç
  Sys_ConsoleInit();
  
  // ADC baslangic
  IADC_IoInit(IOP_JOY_VRX);
  IADC_IoInit(IOP_JOY_VRY);
  IADC_Init(1, ADC_CONT_MODE_ON, ADC_SCAN_MODE_OFF);
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk geçiþ saati
  clock_t t1;           // Güncel saat deðeri
  
  
  t1 = clock();
  
  switch (state) {
  case I_LED_OFF:
      t0 = t1;      
      IO_Write(IOP_LED, 1);     // LED off
      state = S_LED_OFF;
      //break;    
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 0);     // LED On
    state = S_LED_ON;
    //break;    
  case S_LED_ON:
    if (t1 >= t0 + CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
  }  
}

void speedmeter(uint8_t color)
{
  if(color == GREEN_1){
    IO_Write(IOP_LED_GREEN_1, 1);
    IO_Write(IOP_LED_GREEN_2, 0);
    IO_Write(IOP_LED_YELLOW, 0);
    IO_Write(IOP_LED_RED, 0);
  }else if(color == GREEN_2){
    IO_Write(IOP_LED_GREEN_1, 1);
    IO_Write(IOP_LED_GREEN_2, 1);
    IO_Write(IOP_LED_YELLOW, 0);
    IO_Write(IOP_LED_RED, 0);
  }else if(color == YELLOW){
    IO_Write(IOP_LED_GREEN_1, 1);
    IO_Write(IOP_LED_GREEN_2, 1);
    IO_Write(IOP_LED_YELLOW, 1);
    IO_Write(IOP_LED_RED, 0);
  }else if(color == RED){
    IO_Write(IOP_LED_GREEN_1, 1);
    IO_Write(IOP_LED_GREEN_2, 1);
    IO_Write(IOP_LED_YELLOW, 1);
    IO_Write(IOP_LED_RED, 1);
  }else{
    IO_Write(IOP_LED_GREEN_1, 0);
    IO_Write(IOP_LED_GREEN_2, 0);
    IO_Write(IOP_LED_YELLOW, 0);
    IO_Write(IOP_LED_RED, 0);
  }
}

void Task_Joystick(void)
{
  uint8_t temp;
  int x, y, resultx, resulty;
  static int totalx , totaly, n;
  
  uint8_t data_x[3] = {'X', 0, 0};
  uint8_t data_y[3] = {'Y', 0, 0};
  
  x = IADC_Convert(JOY_CH_X);
  y = IADC_Convert(JOY_CH_Y);
  
  totalx += x;
  totaly += y;
  
  if(++n >= NSAMPLES) {
    n = 0;
    
    resultx = totalx / NSAMPLES;
    data_x[1] = (resultx >> 8) & 0x0F; // resultx_high
    data_x[2] = resultx & 0xFF;        // resultx_low
    
    resulty = totaly / NSAMPLES;
    data_y[1] = (resulty >> 8) & 0x0F; // resulty_high
    data_y[2] = resulty & 0xFF;        // resulty_low 
    
    if(data_y[1] == 8 || data_y[1] == 9) 
      speedmeter(GREEN_1);
    else if(data_y[1] == 10 || data_y[1] == 11)
      speedmeter(GREEN_2);
    else if(data_y[1] == 12 || data_y[1] == 13)
      speedmeter(YELLOW);
    else if(data_y[1] == 14 || data_y[1] == 15)
      speedmeter(RED);
    else
      speedmeter(NONE);
    
    /*
    OLED_SetCursor(0, 0);
    printf("x1=%x x2=%x\n\r", data_x[1], data_x[2]);
    printf("y1=%x y2=%x\n\r", data_y[1], data_y[2]);
    printf("x=%4d\n\r", resultx);
    printf("y=%4d\n\r", resulty);
    */
    /////////////////////////////////////////////////
    nrf24_send(data_x);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    temp = nrf24_lastMessageStatus();

    /*
    OLED_SetCursor(4, 0);
    if(temp == NRF24_TRANSMISSON_OK)                  
        printf("X is OK  \n\r");
    else if(temp == NRF24_MESSAGE_LOST)                  
        printf("X is lost\n\r");  
    */
    //nrf24_powerDown();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    nrf24_send(data_y);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    temp = nrf24_lastMessageStatus();

    /*
    OLED_SetCursor(5, 0);
    if(temp == NRF24_TRANSMISSON_OK)                  
        printf("Y is OK  \n\r");
    else if(temp == NRF24_MESSAGE_LOST)                  
        printf("Y is lost\n\r");      
    */
    nrf24_powerDown();
    //////////////////////////////////////////////////
    
    DelayUs(10);
    
    totalx = 0;
    totaly = 0;
  }
}

int main()
{
  uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  
  // Baþlangýç yapýlandýrmalarý
  init();
  
  // Set the device addresses 
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    Task_Joystick();
  }
}


