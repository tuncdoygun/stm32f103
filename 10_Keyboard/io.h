#ifndef _IO_H
#define _IO_H

#include "stm32f10x_gpio.h"

// #define IO_OLED_VDD
#define IO_OLED_RES
#define IO_OLED_CS

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
  
#ifdef IO_OLED_RES
  IOP_OLED_RES,
#endif
  IOP_OLED_DC,
#ifdef IO_OLED_CS
  IOP_OLED_CS,
#endif  

  // Matrix Keyboard
  IOP_KEY_ROW0,
  IOP_KEY_ROW1,
  IOP_KEY_ROW2,
  IOP_KEY_ROW3,
  IOP_KEY_COL0,
  IOP_KEY_COL1,
  IOP_KEY_COL2,
  IOP_KEY_COL3,
};

// Variable definitions
#ifdef _IOS_
IO_PIN _ios[] = {
  { IO_PORT_C, 13 },    // LED
  
  // SSPI
  /*
  { IO_PORT_B, 13 },    // SCK
  { IO_PORT_A, 10 },    // MISO
  { IO_PORT_B, 14 },    // MOSI
  */
  
  // HSPI
  {IO_PORT_A, 5}, // SCK
  {IO_PORT_A, 6}, // MISO
  {IO_PORT_A, 7}, // MOSI
  
  // OLED
#ifdef IO_OLED_VDD
  { IO_PORT_B, 12 },    // SSPI icin 
#endif
  
#ifdef IO_OLED_GND
#endif

#ifdef IO_OLED_RES
  { IO_PORT_B, 0 },    // RES
#endif
  {IO_PORT_B, 1},      // DC
#ifdef IO_OLED_CS
  {IO_PORT_A, 4},      // CS
#endif  
  
  // Matrix Keyboard
  { IO_PORT_A, 8 },    // ROW0
  { IO_PORT_A, 9 },    // ROW1
  { IO_PORT_A, 10 },   // ROW2
  { IO_PORT_A, 11 },   // ROW3
  { IO_PORT_B, 15 },    // COL0
  { IO_PORT_B, 14 },    // COL1
  { IO_PORT_B, 13 },    // COL2
  { IO_PORT_B, 12 },    // COL3  
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