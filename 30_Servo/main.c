#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "uart.h"
#include "spi.h"
#include "nRF24.h"
#include "timer.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portları başlangıç
  Sys_IoInit();
  
  // LED başlangıç
  IO_Write(IOP_LED, 1); // 0 olarak başlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console başlangıç
  Sys_ConsoleInit();
    
  // init hardware pins
  nrf24_init();
    
  // Channel #2 , payload length: 3
  nrf24_config(2, 3);
}

// 29.07.2021
// yukardakine göre daha optimize
void Task_LED(void)
{
  static enum {
    I_LED_OFF,// led_off durumuna geçerken ilk yapılacaklar.başlangıçta yapılacak işler çok fazlaysa, nefes aldırmak için daha faydalı.
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk geçiş saati, t1 güncel saat
  
  t1 = clock(); // bu fonksiyona girdiğinde o andaki saat
  
  switch (state){
  case I_LED_OFF:
    t0 = t1;
    IO_Write(IOP_LED, 0);
    state = S_LED_OFF;
    //break;
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiş demek
      state = I_LED_ON;
    }
    break;
    
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 1);
    state = S_LED_ON;
    //break;
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiş demek
      state = I_LED_OFF;
    }    
    break;
  }
}
//

void Task_Print(void)
{
  static unsigned count;
  
  printf("SAYI:%10u\r", ++count);
}

int g_PWMPeriod;

void Task_Servo(void){
  uint8_t data_array[3];
  int datax, datay, duty_x, duty_y;
  
  if(nrf24_dataReady()){
    nrf24_getData(data_array);
    printf("> ");
    printf("%2X ",data_array[0]);
    printf("%2X ",data_array[1]);
    printf("%2X ",data_array[2]);
    
    if(data_array[0] == 'X') {
      datax = (data_array[1] << 8) | data_array[2];
      
      // Duty cycle joystickten gelen veri ile degisiyor
      duty_x = datax * g_PWMPeriod / 4095; 
      PWM_Duty(duty_x);  
      
      printf("datax = %d duty_x = %d\r\n", datax, duty_x);
    } else if(data_array[0] == 'Y') {
      datay = (data_array[1] << 8) | data_array[2];
      
      /*
      // Duty cycle joystickten gelen veri ile degisiyor
      duty = datay * g_PWMPeriod / 4095; 
      PWM_Duty(duty); 
      */
      
      printf("datay = %d\r\n", datay);
    } else 
      printf("Bilinmeyen veri tipi!\r\n");
  }
}

int main()
{
  uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7}; 
  
  init();
   
  nrf24_tx_address(rx_address);
  nrf24_rx_address(tx_address);
  
  g_PWMPeriod = PWM_Init(10000, 50); 
  //printRadioSettings();
  //printConfigReg();
  //printStatusReg();
  
  while (1)
  {
    Task_LED();  
    Task_Servo();

  }
}


