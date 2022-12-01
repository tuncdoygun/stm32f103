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
  IOP_TEST,     // Test amaçlý(B5)
  IOP_MYINPUT,  // Test amaçlý okuma A3
};

#ifdef _IOS_ // birkere tanýmlanmasý gereken

IO_PIN _ios[] = {
  {IO_PORT_C, 13} ,
  {IO_PORT_B, 5} ,  // Test amaçlý(B5)
  {IO_PORT_A, 3} ,
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
void IO_Toggle(int idx);
int IO_Read(int idx); // burda dönüþ deðerinin int veya bool kullanýlmasýnýn farký yok.32 bitte olsa 1 bitte olsa 1 cycle'da iþliyor iþlemci.

#endif