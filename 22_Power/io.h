#ifndef _IO_H
#define _IO_H

#include "stm32f10x_gpio.h"

#define IO_OLED_RES // kullanýlacak çipte cs veya res olup olmama durumu
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
  
  IOP_BTN0,
  IOP_BTN1,
  
  // UART1
  IOP_U1TX,
  IOP_U1RX,
 
  // UART2
  // IOP_U2TX,
  // IOP_U2RX,

  // UART3
  // IOP_U3TX,
  // IOP_U3RX,
  
  // SPI
  IOP_SPI_SCK,
  IOP_SPI_MISO,
  IOP_SPI_MOSI,
  
  // OLED
#ifdef IO_OLED_VDD
  IOP_OLED_VDD, // yazýlýmsal spi kullanýlacaksa,yani GPIO'ya baðlandýysa
#endif
  
#ifdef IO_OLED_GND
  IOP_OLED_GND, // yazýlýmsal spi kullanýlacaksa,yani GPIO'ya baðlandýysa
#endif  
  
#ifdef IO_OLED_RES
  IOP_OLED_RES,
#endif
  IOP_OLED_DC,  
  
#ifdef IO_OLED_CS
  IOP_OLED_CS,
#endif  
};

#ifdef _IOS_ // birkere tanýmlanmasý gereken

IO_PIN _ios[] = {
  {IO_PORT_C, 13},
  
  { IO_PORT_A, 0 },    // BTN0
  { IO_PORT_A, 2 },    // BTN1
  
  // UART1
  {IO_PORT_A, 9},
  {IO_PORT_A, 10},
  
  // UART2
  //{IO_PORT_A, 2},
  //{IO_PORT_A, 3},

  // UART3
  // {IO_PORT_B, 10},
  // {IO_PORT_B, 11},
  
  // SPI1
  {IO_PORT_A, 5}, // SCK
  {IO_PORT_A, 6}, // MISO
  {IO_PORT_A, 7}, // MOSI
  
  // OLED
#ifdef IO_OLED_VDD
#endif
  
#ifdef IO_OLED_GND
#endif
  
#ifdef IO_OLED_RES  
  {IO_PORT_B, 0}, // RES
#endif
  {IO_PORT_B, 1}, // DC
#ifdef IO_OLED_CS
  {IO_PORT_A, 4}, // CS
#endif
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

extern IO_PIN _ios[];
void IO_Init(int idx, int mode);
void IO_Write(int idx, int val);
int IO_Read(int idx); // burda dönüþ deðerinin int veya bool kullanýlmasýnýn farký yok.32 bitte olsa 1 bitte olsa 1 cycle'da iþliyor iþlemci.

#endif
