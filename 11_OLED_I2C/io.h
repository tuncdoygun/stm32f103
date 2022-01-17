#ifndef _IO_H
#define _IO_H

#include "stm32f10x_gpio.h"

//#define IO_OLED_VDD
//#define IO_OLED_GND

enum {
  IO_PORT_A,
  IO_PORT_B,
  IO_PORT_C,
  IO_PORT_D,
  IO_PORT_E,
  IO_PORT_F,
  IO_PORT_G,
};

enum {
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
  IOP_LED,      // Blue pill LED
  
  // I2C
  IOP_I2C1_SCL,
  IOP_I2C1_SDA,
  IOP_I2C2_SCL,
  IOP_I2C2_SDA,
  
  // SPI
  IOP_SPI_SCK,
  IOP_SPI_MISO,
  IOP_SPI_MOSI,
  
  // OLED
#ifdef IO_OLED_VDD
  IOP_OLED_VDD,
#endif
  
#ifdef IO_OLED_GND
  IOP_OLED_GND,
#endif
};

// Variable definitions
#ifdef _IOS_
IO_PIN _ios[] = {
  { IO_PORT_C, 13 },    // LED
  
  // I2C
  { IO_PORT_B, 6 },    // SCL1
  { IO_PORT_B, 7 },    // SDA1
  { IO_PORT_B, 10 },   // SCL2
  { IO_PORT_B, 11 },   // SDA2

  // SPI
  { IO_PORT_A, 5 },    // SCK
  { IO_PORT_A, 6 },    // MISO
  { IO_PORT_A, 7 },    // MOSI

  // OLED
#ifdef IO_OLED_VDD
  { IO_PORT_B, 1 },   // OLED VDD
#endif
  
#ifdef IO_OLED_GND
  { IO_PORT_B, 0 },   // OLED GND
#endif
};

GPIO_TypeDef    *_GPIO_Ports[] = {
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
int IO_Read(int idx);

#endif