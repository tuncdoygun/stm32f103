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

int g_PWMPeriod;

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
  DelayMs(10);
  
  // 50Hz %7 pwm start
  g_PWMPeriod = PWM_Init(50, 7, TIM2_CH_3); // ESC
  g_PWMPeriod = PWM_Init(50, 7, TIM2_CH_2); // SERVO
  
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

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Task_Servo(void){
  uint8_t data_array[3];
  static char debug_str_x[100], debug_str_y[100];
  int datax, datay, duty_x, duty_y;
  
   if(nrf24_dataReady()){
    nrf24_getData(data_array);
    /*
    printf("> ");
    printf("%2X ",data_array[0]);
    printf("%2X ",data_array[1]);
    printf("%2X ",data_array[2]);
    */
    
    if(data_array[0] == 'X') {
      datax = (data_array[1] << 8) | data_array[2];
      
      // Duty cycle joystickten gelen veri ile degisiyor
      //duty_x = datax * g_PWMPeriod / 4095; 
      
      /*
      Normalde duty yukardaki gibi hesaplaniyor.Fakat servo motorun calismasi icin gereken duty'i
      joystick'in tamamina yayabilmek icin maplamak gerekiyor.
      Gelen 12 bitlik adc datasini(normalde 0-4095 fakat 60-4000 aldik) bir altindaki formule gore(bu pwm yazilimina gore servo
      duty araligi 550-3800) mapladik.
   
      550 0 derece (0,11V) 
      2150 90 derece (0,35V)
      3800 180 derece (0,6V)
      */
      datax = map(datax, 60, 4000, 156, 380);
      datax = 536 - datax; // joystick sağa doğru iken tekerlek sola döndüğünden terslendi. 
      duty_x = datax * g_PWMPeriod / 4095;
      
      PWM_Duty(duty_x, TIM2_CH_2);  
      
      //printf("datax = %d duty_x = %d g_PWMPeriod = %d\r\n", datax, duty_x, g_PWMPeriod);
      sprintf(debug_str_x, "> [%c] %2X %2X datax = %d", data_array[0], data_array[1], data_array[2], datax);  
    } else if(data_array[0] == 'Y') {
      datay = (data_array[1] << 8) | data_array[2];
      
      //datay = map(datay, 60, 4000, 260, 320); 
      datay = map(datay, 60, 4000, 282, 302); 
      duty_y = datay * g_PWMPeriod / 4095;
      
      PWM_Duty(duty_y, TIM2_CH_3); 
      
      //printf("datay = %d duty_y = %d g_PWMPeriod = %d\r\n", datay, duty_y, g_PWMPeriod);
      sprintf(debug_str_y, "[%c] %2X %2X datay = %d", data_array[0], data_array[1], data_array[2], datay); 
      printf("%s\n", strcat(debug_str_x, debug_str_y)); 
    } else 
      printf("Bilinmeyen veri tipi!\r\n");
    
  }
}

int main()
{
  uint8_t tx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  uint8_t rx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7}; 
  
  init();
  
  printf("Hello SERVO\n");
  
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);
  
  //PWM_Duty(3500, TIM2_CH_3);
  //PWM_Duty(SERVO_0, TIM2_CH_2);
  
  //DelayMs(1000);
  //PWM_Duty(4000, TIM2_CH_3);
  
  int i;
  /*
  PWM_Duty(SERVO_90, TIM2_CH_2);
  
  for(i = 1; i < 10; ++i){
    DelayMs(50);
   PWM_Duty(SERVO_90 + i * 100, TIM2_CH_2);
  }
  
   for(i = 1; i < 10; ++i){
    DelayMs(50);
   PWM_Duty(SERVO_90 - i * 100, TIM2_CH_2);
   }
  */
  //DelayMs(1000);
  //PWM_Duty(4500, TIM2_CH_3);
  //PWM_Duty(SERVO_180, TIM2_CH_2);
  
  printRadioSettings();
  //printConfigReg();
  //printStatusReg();
  
  while (1)
  {
    Task_LED();  
    Task_Servo();

  }
}


