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

#define DEBUGg

#define HIGH 1
#define LOW  0

#define DIKEY_MIN 900
#define DIKEY_MAX 4100
#define YATAY_MIN 900
#define YATAY_MAX 4100

typedef struct {
  int duty;
  int flag;
  int pwm_timer;
}MOTOR;

uint8_t data_array[6];
int datax, datay;
int servo_duty_dikey = DIKEY_MIN;
int servo_duty_yatay = YATAY_MIN;

MOTOR sag_motor = {0, LOW, TIM4_CH_1};
MOTOR sol_motor = {0, LOW, TIM4_CH_2};

int g_PWMPeriod;

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1); // 0 olarak ba�latmak i�in
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  IO_Write(IOP_LED_KIRMIZI, 0);
  IO_Init(IOP_LED_KIRMIZI, IO_MODE_OUTPUT);
  IO_Write(IOP_LED_BEYAZ, 0);
  IO_Init(IOP_LED_BEYAZ, IO_MODE_OUTPUT);  
  
  // IN1 IN2 baslangic
  IO_Write(IOP_IN1, 1);
  IO_Init(IOP_IN1, IO_MODE_OUTPUT);
  IO_Write(IOP_IN2, 0);
  IO_Init(IOP_IN2, IO_MODE_OUTPUT); 
  IO_Write(IOP_IN3, 0);
  IO_Init(IOP_IN3, IO_MODE_OUTPUT);
  IO_Write(IOP_IN4, 1);
  IO_Init(IOP_IN4, IO_MODE_OUTPUT);
  
  //IO_Write(IOP_PWM_DC_2, 1); 
  //IO_Write(IOP_PWM_DC_1, 1);  
  
  // Console ba�lang��
  Sys_ConsoleInit();
    
  // init hardware pins
  nrf24_init();
    
  // Channel #2 , payload length: 3
  nrf24_config(2, 6);
  DelayMs(10);
  
  // 50Hz %7 pwm start, ka�tan ba�lat�laca�� bak�lacak
  g_PWMPeriod = PWM_Init(50, 0, TIMER_2, TIM2_CH_4); // servo 2
  g_PWMPeriod = PWM_Init(50, 0, TIMER_2, TIM2_CH_3); // servo 1
  g_PWMPeriod = PWM_Init(50, 0, TIMER_4, TIM4_CH_2); // dc motor 2
  g_PWMPeriod = PWM_Init(50, 0, TIMER_4, TIM4_CH_1); // dc motor 1  
}

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

int map(int x, int in_min, int in_max, int out_min, int out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void clear_buffer(void) {
  data_array[0] = 0;
  data_array[1] = 0;
  data_array[2] = 0;
}

void Task_NRF(void) {
  if(nrf24_dataReady()){
   nrf24_getData(data_array);
   //printf("%c %c %x %x %x %x\n", data_array[0], data_array[1], data_array[2], data_array[3], data_array[4], data_array[5]);
  }
}

void Task_DC(void){ 
  if(data_array[0] == 'X' && data_array[1] == 'Y') { 
    // joystick X bolgesi sadece saga/sola donerken motor hizlarini azaltmak icin kullanilir.
    datax = (data_array[2] << 8) | data_array[3];
    datax = map(datax, 0, 3800, 0, 1000);

    datay = (data_array[4] << 8) | data_array[5];
    datay = map(datay, 0, 3800, 0, 1000); 
    
    if(datax >= 530 && datax <= 1050) 
      sag_motor.flag = HIGH;
    else if (datax >= 0 && datax <= 430)
      sol_motor.flag = HIGH;
    
#ifdef DEBUG
    //printf("datax = %4d datay = %4d SAG_FLAG = %4s SOL_FLAG = %4s \n", datax, datay, sag_motor.flag ? "HIGH" : "LOW", sol_motor.flag ? "HIGH" : "LOW"); 
    //printf("datax = %4d datay = %4d \n", datax, datay); 
#endif 
    
    if(datay >= 550 || datax >= 530 || datax <= 430){ // joystick sabit bolge disi. 
      if(sag_motor.flag){
        // joystick x eksenindeki datax 550-1000 arasindaysa,yani saga cevrildiyse,sag motor hizi dusecek.
        //sag_motor.duty = (datay - datax ) * g_PWMPeriod / 4095;
        //sag_motor.duty += 4500;
        
        if(datay <= 550) // y ekseni degeri sabit bolgedeyse,joystick sadece X degerinde hareket ediyorsa saga/sola donmek icin
          datay = 800;   // deger yetmiyor.O sebepten 800 e sabitlenmeli.
        
        sol_motor.duty = datay * g_PWMPeriod / 4095; 
        sag_motor.duty = 0;  
        sag_motor.flag = LOW;
      } else if(sol_motor.flag) {
        // joystick y eksenindeki datay 0-400 arasindaysa,yani sola cevrildiyse,sol motor hizi dusecek.
        //sag_motor.duty = datay * g_PWMPeriod / 4095;
        //sol_motor.duty = (datay - (1000 - datax)) * g_PWMPeriod / 4095; // 1000'den cikarma sebebi x ekseninde sola dogru gittikce deger azaliyor(400'den 0'a dogru.)
        //sol_motor.duty += 4500;      
                                                           // x ekseni degeri 400 iken datay'den 600 cikarilmali ki sag ve sol motor yavaslamalari ayni olsun.
        
        if(datay <= 550) // y ekseni degeri sabit bolgedeyse,joystick sadece X degerinde hareket ediyorsa saga/sola donmek icin
          datay = 800;   // deger yetmiyor.O sebepten 800 e sabitlenmeli.
        
        sag_motor.duty = datay * g_PWMPeriod / 4095;
        sol_motor.duty = 0;
        sol_motor.flag = LOW;
      } else {
        // joystick x ekseni 430-530 arasindaysa,yani orta noktadaysa.Motorlar y ekseni degerlerine gore 
        // ya tamamen duracak ya da esit hizlarla hizlanip/yavaslayacak.
        sag_motor.duty = sol_motor.duty = datay * g_PWMPeriod / 4095;
      }      
    } else { // joystick sabitken(sabit b�lgedeyken) durmasi icin.
      sag_motor.duty = 0;
      sol_motor.duty = 0; 
    }
    
    PWM_Duty(sag_motor.duty, sag_motor.pwm_timer); 
    PWM_Duty(sol_motor.duty, sol_motor.pwm_timer);
    
#ifdef DEBUG
    //printf("sag_motor.duty = %4d sol_motor.duty = %4d SAG_FLAG = %4s SOL_FLAG = %4s \n", sag_motor.duty, sol_motor.duty, sag_motor.flag ? "HIGH" : "LOW", sol_motor.flag ? "HIGH" : "LOW");
    //printf("sag_motor.duty = %4d sol_motor.duty = %4d \n", sag_motor.duty, sol_motor.duty);
#endif  
  }
}

void Task_DC__(void){
  if(data_array[0] == 'X') { 
    // joystick X bolgesi sadece saga/sola donerken motor hizlarini azaltmak icin kullanilir.
    datax = (data_array[1] << 8) | data_array[2];
    datax = map(datax, 0, 4000, 0, 1000);
     
    if(datax >= 600 && datax <= 1000) // sag motor hizi ayni oranda azalacak.
      sag_motor.flag = HIGH;
    else if (datax >= 0 && datax <= 400) // sol motor hizi ayni oranda azalacak.
      sol_motor.flag = HIGH;

#ifdef DEBUG
    printf("\ndatax = %4d SAG_FLAG = %4s SOL_FLAG = %4s ", datax, sag_motor.flag ? "HIGH" : "LOW", sol_motor.flag ? "HIGH" : "LOW");
#endif   
  }
    
  if(data_array[0] == 'Y') { // HIZ
    datay = (data_array[1] << 8) | data_array[2];
    datay = map(datay, 0, 4000, 0, 700);
      
#ifdef DEBUG
    printf("datay = %4d ", datay);
#endif   
      
    if(datay >= 550 || datax >= 600 || datax <= 400){ // joystick sabit bolge disi. 
      if(sag_motor.flag){
      // joystick x eksenindeki datax 550-1000 arasindaysa,yani saga cevrildiyse,sag motor hizi dusecek.
      sag_motor.duty = (datay - datax ) * g_PWMPeriod / 4095;
      //sag_motor.duty += 4500;
      sol_motor.duty = datay * g_PWMPeriod / 4095; 
          
      sag_motor.flag = LOW;
      } else if(sol_motor.flag) {
      // joystick y eksenindeki datay 0-400 arasindaysa,yani sola cevrildiyse,sol motor hizi dusecek.
      sag_motor.duty = datay * g_PWMPeriod / 4095;
      sol_motor.duty = (datay - (1000 - datax)) * g_PWMPeriod / 4095; // 1000'den cikarma sebebi x ekseninde sola dogru gittikce deger azaliyor(400'den 0'a dogru.)
      //sol_motor.duty += 4500;                                                                 // x ekseni degeri 400 iken datay'den 600 cikarilmali ki sag ve sol motor yavaslamalari ayni olsun.
         
      sol_motor.flag = LOW;
      } else {
      // joystick x ekseni 400-550 arasindaysa,yani orta noktadaysa.Motorlar ya tamamen duracak ya da esit hizlarla hizlanip/yavaslayacak.
      sag_motor.duty = sol_motor.duty = datay * g_PWMPeriod / 4095;
      }
        
#ifdef DEBUG
    printf("SA_DUTY = %4d SO_DUTY = %4d", sag_motor.duty, sol_motor.duty);
#endif           
      PWM_Duty(sag_motor.duty, sag_motor.pwm_timer);
      //PWM_Duty(sol_motor.duty, sol_motor.pwm_timer);
    } else { // joystick sabitken durmasi icin.
      PWM_Duty(0, sag_motor.pwm_timer); 
      PWM_Duty(0, sol_motor.pwm_timer);        
    }
  }
}

void Task_Servo(void){
  if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'U') {
    if(servo_duty_dikey < DIKEY_MAX){
      servo_duty_dikey += 20;
      PWM_Duty(servo_duty_dikey, TIM2_CH_3); 
    
#ifdef DEBUG
      printf("BUTTON UP servo_duty_dikey = %d\n", servo_duty_dikey);
#endif
    }
  } else if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'D') {
    if(servo_duty_dikey > DIKEY_MIN) {
      servo_duty_dikey -= 20;
      PWM_Duty(servo_duty_dikey, TIM2_CH_3); 
      
#ifdef DEBUG
      printf("BUTTON DOWN servo_duty_dikey = %d\n", servo_duty_dikey);
#endif
    }
  }
  
  if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'R') {
    if(servo_duty_yatay < YATAY_MAX){
      servo_duty_yatay += 20;
      PWM_Duty(servo_duty_yatay, TIM2_CH_4);
    
#ifdef DEBUG
      printf("BUTTON RIGHT servo_duty_yatay = %d\n", servo_duty_yatay);
#endif
    }
  } else if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'L') {
    if(servo_duty_yatay > YATAY_MIN) {    
      servo_duty_yatay -= 20;
      PWM_Duty(servo_duty_yatay, TIM2_CH_4);
    
#ifdef DEBUG
      printf("BUTTON LEFT servo_duty_yatay = %d\n", servo_duty_yatay);
#endif
    }
  }  
}

void Task_tank_led(void){
  if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'J') {
    IO_Write(IOP_LED_KIRMIZI, 1);
    IO_Write(IOP_LED_BEYAZ, 1);
    
#ifdef DEBUG
    printf("LEDS ON\n");
#endif
  } else if(data_array[0] == 'B' && data_array[1] == 'T' && data_array[2] == 'M') {
    IO_Write(IOP_LED_KIRMIZI, 0);
    IO_Write(IOP_LED_BEYAZ, 0);
    
#ifdef DEBUG
    printf("LEDS OFF\n");
#endif
  }
  clear_buffer();
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
  PWM_Duty(5000, TIM4_CH_1);
  PWM_Duty(5000, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(6000, TIM4_CH_1);
  PWM_Duty(6000, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(7300, TIM4_CH_1);
  PWM_Duty(7300, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(8000, TIM4_CH_1);
  PWM_Duty(8000, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(9000, TIM4_CH_1);
  PWM_Duty(9000, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(10000, TIM4_CH_1);
  PWM_Duty(10000, TIM4_CH_2);
  //DelayMs(5000);
  
  PWM_Duty(11000, TIM4_CH_1);
  PWM_Duty(11000, TIM4_CH_2);  
  
  //PWM_Duty(SERVO_180, TIM2_CH_3);
  //PWM_Duty(SERVO_180, TIM2_CH_4);
  */
  
  //printRadioSettings();
  //printConfigReg();
  //printStatusReg();
  
  while (1)
  {
    Task_LED();
    Task_NRF();
    Task_DC();
    Task_Servo();
    Task_tank_led();
  }
}


