#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
#include "adc.h"
#include "nRF24.h"

#define SYS_CLOCK_FREQ  72000000
#define JOY_CH_X        0
#define JOY_CH_Y        1

#define NSAMPLES        1000

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar? ba?lang??
  Sys_IoInit();
  
  // LED ba?lang??
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console(oled) ba?lang??
  Sys_ConsoleInit();
  
  // ADC baslangic
  IADC_IoInit(IOP_JOY_VRX);
  IADC_IoInit(IOP_JOY_VRY);
  IADC_Init(1, ADC_CONT_MODE_ON, ADC_SCAN_MODE_OFF);
  
  // nRF24 baslangic
  nrf24_init();  
  nrf24_config(2, 3);  // Channel #2 , payload length: 3
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk ge?i? saati
  clock_t t1;           // G?ncel saat de?eri
  
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
    
    OLED_SetCursor(0, 0);
    printf("x1=%x x2=%x\n", data_x[1], data_x[2]);
    printf("y1=%x y2=%x\n", data_y[1], data_y[2]);
    printf("x=%d y=%d\n", resultx, resulty);
    
    /////////////////////////////////////////////////
    nrf24_send(data_x);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    temp = nrf24_lastMessageStatus();

    OLED_SetCursor(3, 0);
    if(temp == NRF24_TRANSMISSON_OK)                  
        printf("X is OK\n");
    else if(temp == NRF24_MESSAGE_LOST)                  
        printf("X is lost\n");  
    
    nrf24_powerDown();
    //////////////////////////////////////////////////
    
    //////////////////////////////////////////////////
    nrf24_send(data_y);
    while(nrf24_isSending());
    /* Make analysis on last tranmission attempt */
    temp = nrf24_lastMessageStatus();

    OLED_SetCursor(4, 0);
    if(temp == NRF24_TRANSMISSON_OK)                  
        printf("Y is OK\n");
    else if(temp == NRF24_MESSAGE_LOST)                  
        printf("Y is lost\n");      
    
    nrf24_powerDown();
    //////////////////////////////////////////////////
    
    totalx = 0;
    totaly = 0;
  }
}

int main()
{
  uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  
  // Ba?lang?? yap?land?rmalar?
  init();

  OLED_SetFont(FNT_SMALL);
  
  // Set the device addresses 
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);
  
  // G?rev ?evrimi (Task Loop)
  // Co-Operative Multitasking (Yard?mla?mal? ?oklu g?rev) 
  while (1)
  {
    Task_LED();
    Task_Joystick();
  }
}


