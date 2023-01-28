#ifndef _IO_H
#define _IO_H

#include "stm32f10x_gpio.h"

enum {
  IO_PORT_A,
  IO_PORT_B,
  IO_PORT_C,
  IO_PORT_D,
  IO_PORT_E,
  IO_PORT_F,
  IO_PORT_G,
};

enum { // stm32f10x_gpio.h GPIOMode_TypeDef struct
 IO_MODE_INPUT = GPIO_Mode_IN_FLOATING,
 IO_MODE_OUTPUT = GPIO_Mode_Out_PP,
 IO_MODE_ALTERNATE = GPIO_Mode_AF_PP,
 IO_MODE_ANALOG = GPIO_Mode_AIN,
 
 IO_MODE_INPUT_PD = GPIO_Mode_IPD,
 IO_MODE_INPUT_PU = GPIO_Mode_IPU,
 IO_MODE_OUTPUT_OD = GPIO_Mode_Out_OD,
 IO_MODE_ALTERNATE_OD = GPIO_Mode_AF_OD,
};

typedef struct {
  int port;
  int pin;
} IO_PIN;

enum {
  IOP_LED,      // Bluepill LED (C13)
  
  // SPI1
  IOP_SPI_SCK,
  IOP_SPI_MISO,
  IOP_SPI_MOSI,
  
  // SPI2
  IOP_SPI2_SCK,
  IOP_SPI2_MISO,
  IOP_SPI2_MOSI,
  
  // nRF24
  IOP_nRF_CSN,
  IOP_nRF_CE,
  
  // UART1-console
  IOP_U1RX,
  IOP_U1TX,
  
  // TIM2_CH3 pwm cikisi
  IOP_PWM_ESC,
  
  // TIM2_CH2 pwm cikisi
  IOP_PWM_SERVO,  
};

#ifdef _IOS_ // birkere tan�mlanmas� gereken

IO_PIN _ios[] = {
  {IO_PORT_C, 13},
  
  // SPI1
  {IO_PORT_A, 5}, // SCK
  {IO_PORT_A, 6}, // MISO
  {IO_PORT_A, 7}, // MOSI
  
  // SPI2
  { IO_PORT_B, 13 },    // SCK
  { IO_PORT_B, 14 },    // MISO
  { IO_PORT_B, 15 },    // MOSI  

  // nRF24
  {IO_PORT_A, 4}, // CSN
  {IO_PORT_A, 3}, // CE  
  
  // UART-1
  {IO_PORT_A, 10}, // RX
  {IO_PORT_A, 9},  // TX
  
  // TIM2_CH3 PWM cikisi
  {IO_PORT_A, 2}, // ESC

  // TIM2_CH2 PWM cikisi
  {IO_PORT_A, 1}, // servo  
};

GPIO_TypeDef *_GPIO_Ports[] = {
  GPIOA,
  GPIOB,
  GPIOC,
  GPIOD,
  GPIOE,
  GPIOF,
  GPIOG,
};

#endif

void IO_Init(int idx, int mode);
void IO_Write(int idx, int val);
int IO_Read(int idx); // burda d�n�� de�erinin int veya bool kullan�lmas�n�n fark� yok.32 bitte olsa 1 bitte olsa 1 cycle'da i�liyor i�lemci.

#endif