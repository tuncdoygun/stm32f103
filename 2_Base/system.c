#include <stdio.h>
#include <stdint.h>

#include "stm32f10x.h"
#include "system.h"

static volatile clock_t _tmTick;

static const int _GPIO_Clocks[] = {
  RCC_APB2Periph_GPIOA,
  RCC_APB2Periph_GPIOB,
  RCC_APB2Periph_GPIOC,
  RCC_APB2Periph_GPIOD,
  RCC_APB2Periph_GPIOE,
  RCC_APB2Periph_GPIOF,
  RCC_APB2Periph_GPIOG,
};

#define N_PORTS (sizeof(_GPIO_Clocks) / sizeof(*_GPIO_Clocks))

void Sys_IoInit(void)
{
  int i;
  
  for(i = 0; i < N_PORTS; ++i) // B�t�n GPIO Clock i�aretlerinin a��lmas�.Extreme g�� tasarrufu gerekmedi�inden hepsini enable etmek bir�ey de�i�tirmez
    RCC_APB2PeriphClockCmd(_GPIO_Clocks[i], ENABLE);
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // AFIO Clock
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE); // JTAG disable.Biz STLINK kulland���m�zdan fazla pin kullanan JTAG'i disable ettik.
}

void Sys_ClockTick(void)
{
  // elapsed time counter.1 ms'de bir de�er 1 artacak.
  ++_tmTick;
}

clock_t clock(void)
{
  return _tmTick;
}

void Sys_ClockInit(void)
{
  SysTick_Config(SystemCoreClock / CLOCKS_PER_SEC); // buraya verilen de�er geriye sar�l�yor.0'a ula��nca kesme oluyor.
                                                    // Buradaki SystemCoreClock de�eri CLOCKS_PER_SEC'a b�l�nmeseydi 1 sn'de bir kesme olu�urdu.
                                                    // ��nk� SystemCoreClock = 72 MHz, 1 saniyede 72 milyon pulse, 1 saniye sonra 0'a d��er de�er.
                                                    // Biz burda CLOCKS_PER_SEC'a b�ld�k b�ylece 1ms'de bir kesme olacak. 
  
                                                    // Kesme olu�tu�unda stm32f10x_it.c'deki SysTick_Handler fonksiyonu �al���r.
}

