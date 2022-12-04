#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "adc.h"
#include "nRF24.h"
#include "button.h"

#define SYS_CLOCK_FREQ  72000000
#define JOY_CH_HIZ        0
#define JOY_CH_YON        1

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
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Joystick button baslangic
  BTN_InitButtons();
  
  // Console(oled) ba�lang��
  Sys_ConsoleInit();
  
  // init hardware pins
  nrf24_init();
    
  // Channel #2 , payload length: 3
  nrf24_config(2, 3);
  DelayMs(10);  
  
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
  
  static clock_t t0;    // Duruma ilk ge�i� saati
  clock_t t1;           // G�ncel saat de�eri
  
  
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

/*
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
*/

void Task_Joystick(void)
{
  uint8_t temp;
  int x, y, resultx, resulty;
  static int totalx , totaly, n;
  
  uint8_t data_x[3] = {'X', 0, 0};
  uint8_t data_y[3] = {'Y', 0, 0};
  
  x = IADC_Convert(JOY_CH_YON);
  y = IADC_Convert(JOY_CH_HIZ);
  
  totalx += x;
  totaly += y;
  
  if(++n >= NSAMPLES) {
    n = 0;
    
    resultx = totalx / NSAMPLES;
    data_x[1] = (resultx >> 8) & 0x0F; // resultx_high
    data_x[2] = resultx & 0xFF;        // resultx_low
    
    resulty = totaly / NSAMPLES;
    data_y[1] = (resulty >> 8) & 0x0F; // resulty_high

    //printf("x1=%x x2=%x y1=%x y2=%x x=%4d y=%4d\n\r", data_x[1], data_x[2], data_y[1], data_y[2], resultx, resulty);
    
    //////////////////////////////////////////////////
    nrf24_send(data_x);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    //temp = nrf24_lastMessageStatus();
    //////////////////////////////////////////////////

    //////////////////////////////////////////////////
    nrf24_send(data_y);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    //temp = nrf24_lastMessageStatus();
    
    //////////////////////////////////////////////////
    
    nrf24_powerDown();
    DelayUs(10);
    
    totalx = 0;
    totaly = 0;
  }
}

void Task_Button(void)
{
    uint8_t data[3] = {'B', 'T', 0};
  
  if (g_Buttons[BTN_UP]){
    printf("BTN_UP\n");
    
    data[2] = 'U';
    nrf24_send(data);
    while(nrf24_isSending());    
    
    data[2] = 0;
    g_Buttons[BTN_UP] = 0; //binary semaphore
  }

  if (g_Buttons[BTN_DOWN]){
    printf("BTN_DOWN\n");
    
    data[2] = 'D';
    nrf24_send(data);
    while(nrf24_isSending());    
    
    data[2] = 0;
    g_Buttons[BTN_DOWN] = 0; //binary semaphore
  }

  if (g_Buttons[BTN_RIGHT]){
    printf("BTN_RIGHT\n");
 
    data[2] = 'R';
    nrf24_send(data);
    while(nrf24_isSending());    
    
    data[2] = 0;
    g_Buttons[BTN_RIGHT] = 0; //binary semaphore
  }

  if (g_Buttons[BTN_LEFT]){
    printf("BTN_LEFT\n");
    
    data[2] = 'L';
    nrf24_send(data);
    while(nrf24_isSending());    
    
    data[2] = 0;
    g_Buttons[BTN_LEFT] = 0; //binary semaphore
  }
  
#ifdef BTN_LONG_PRESS
  if (g_ButtonsL[BTN_JOY]){
    printf("BTN_JOY\n");
    
    data[2] = 'J';
    nrf24_send(data);
    while(nrf24_isSending());    
    
    data[2] = 0;  
    g_ButtonsL[BTN_JOY] = 0; //binary semaphore
  }
#endif
}

int main()
{
  uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  
  // Ba�lang�� yap�land�rmalar�
  init();
  
  printf("********** PROTOTANK KUMANDA **********\n");
  printf("Sistem baslatiliyor...\n");
  
  // Set the device addresses 
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);
  printf("TX Address : E7E7E7E7E7\n");
  printf("RX Address : D7D7D7D7D7\n");
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev) 
  while (1)
  {
    Task_LED();
    Task_Button();
    Task_Joystick();
  }
}


