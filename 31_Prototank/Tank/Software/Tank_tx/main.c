#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "uart.h"
#include "spi.h"
#include "nRF24.h"
#include "timer.h"

#define SERVO_0                 550
#define SERVO_RIGHT_MAX         1250
#define SERVO_90                2150
#define SERVO_LEFT_MAX          3050
#define SERVO_180               3800

#define DEBUG

int g_PWMPeriod;
uint8_t data_array[3];
char debug_str_x[100], debug_str_y[100];
int datax, datay, duty_x, duty_y;
  
void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1); // 0 olarak baþlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  IO_Write(IOP_LED_KIRMIZI, 0);
  IO_Init(IOP_LED_KIRMIZI, IO_MODE_OUTPUT);
  IO_Write(IOP_LED_BEYAZ, 0);
  IO_Init(IOP_LED_BEYAZ, IO_MODE_OUTPUT);  
  
  // Console baþlangýç
  Sys_ConsoleInit();
    
  // init hardware pins
  nrf24_init();
    
  // Channel #2 , payload length: 3
  nrf24_config(2, 3);
  DelayMs(10);
  
  // 50Hz %7 pwm start, kaçtan baþlatýlacaðý bakýlacak
  //g_PWMPeriod = PWM_Init(50, 7, TIM2_CH_4); // servo 2
  //g_PWMPeriod = PWM_Init(50, 7, TIM2_CH_3); // servo 1
  //g_PWMPeriod = PWM_Init(50, 7, TIM2_CH_2); // dc motor 2
  //g_PWMPeriod = PWM_Init(50, 50, TIM2_CH_1); // dc motor 1  
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,// led_off durumuna geçerken ilk yapýlacaklar.baþlangýçta yapýlacak iþler çok fazlaysa, nefes aldýrmak için daha faydalý.
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk geçiþ saati, t1 güncel saat
  
  t1 = clock(); // bu fonksiyona girdiðinde o andaki saat
  
  switch (state){
  case I_LED_OFF:
    t0 = t1;
    IO_Write(IOP_LED, 0);
    state = S_LED_OFF;
    //break;
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiþ demek
      state = I_LED_ON;
    }
    break;
    
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 1);
    state = S_LED_ON;
    //break;
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye geçmiþ demek
      state = I_LED_OFF;
    }    
    break;
  }
}
//

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void clear_buffer(void) {
  data_array[0] = 0;
  data_array[1] = 0;
  data_array[2] = 0;
}

void Task_NRF(void){
   if(nrf24_dataReady()){
    nrf24_getData(data_array);
    
#ifdef DEBUG
    printf("> ");
    printf("%x ",data_array[0]);
    printf("%x ",data_array[1]);
    printf("%x\n",data_array[2]);
#endif
  }
}

void Task_tank_led(void){
  if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'J') {
    IO_Toggle(IOP_LED_BEYAZ);
    IO_Toggle(IOP_LED_KIRMIZI);
    
#ifdef DEBUG
    printf("LEDS %s\n", IO_Read(IOP_LED_BEYAZ) ? "ON" : "OFF");
#endif
    
    clear_buffer();
  }  
}

int main()
{
  uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7}; 
  
  init();
  
  printf("Hello PROTOTANK\n");
  
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);
  
  /*
  PWM_Duty(25000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(26000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(27000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(28000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(29000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(30000, TIM2_CH_1);
  DelayMs(2000);
  PWM_Duty(31000, TIM2_CH_1);
  */
  //printRadioSettings();
  //printConfigReg();
  //printStatusReg();
  
  while (1)
  {
    Task_LED();  
    Task_NRF();
    Task_tank_led();
  }
}


