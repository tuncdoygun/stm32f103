#include <stdint.h>

#define PERIPH_BASE           ((uint32_t)0x40000000) /*!< Peripheral base address in the alias region */
#define APB1PERIPH_BASE       PERIPH_BASE
#define APB2PERIPH_BASE       (PERIPH_BASE + 0x10000)
#define AHBPERIPH_BASE        (PERIPH_BASE + 0x20000)

#define RCC_BASE              (AHBPERIPH_BASE + 0x1000)
#define GPIOC_BASE            (APB2PERIPH_BASE + 0x1000)   // 0x40011000

#define IOP_LED_PORT   GPIOC
#define IOP_LED_PIN    13

#define     __IO    volatile             /*!< Defines 'read / write' permissions */

typedef struct
{
/* 0 */  __IO uint32_t CR;
/* 4 */  __IO uint32_t CFGR;
/* 8 */   __IO uint32_t CIR;
  __IO uint32_t APB2RSTR;
  __IO uint32_t APB1RSTR;
  __IO uint32_t AHBENR;
  __IO uint32_t APB2ENR;
  __IO uint32_t APB1ENR;
  __IO uint32_t BDCR;
  __IO uint32_t CSR;
} RCC_TypeDef;

typedef struct
{
/* 0 */  __IO uint32_t CRL;
/* 4 */  __IO uint32_t CRH;
/* 8 */  __IO uint32_t IDR;
/* 12 */  __IO uint32_t ODR;
  __IO uint32_t BSRR;
  __IO uint32_t BRR;
  __IO uint32_t LCKR;
} GPIO_TypeDef;

#define  RCC_APB2ENR_IOPCEN                  ((uint32_t)0x00000010)         /*!< I/O port C clock enable */

#define GPIOC               ((GPIO_TypeDef *) GPIOC_BASE)
#define RCC                 ((RCC_TypeDef *) RCC_BASE)

void Wait(void)
{
}

void Delay(void)
{
  int i;
  
  for (i = 0; i < 1000000; ++i)
    Wait();
}

// Ama�: PC13'e ba�l� LED'� yak�p s�nd�rmek. Bunun i�in PC13 ��k��-0 ��k��-1 yap�lmal�
// Not: Bu uygulama debug ortam�nda ad�m ad�m �al��t�r�lmal�d�r. Program ko�turuldu�unda
// yanma/s�nme tepki ve alg� s�releri �ok k�sa olaca��ndan durum anla��lmayacakt�r.
int main()
{
  int bitOffset;
  
  // 1) Saat i�aretini a�
  // Reset ve Clock Control register'� clock sa�lamak i�in kullan�lacak
  RCC->APB2ENR |= RCC_APB2ENR_IOPCEN;   // GPIOC clock enable
  
  // 2) Portu yap�land�r
  GPIOC->BSRR = (1 << IOP_LED_PIN);     // DR<13> = 1
  
  //GPIOC->BSRR = 1 << (IOP_LED_PIN * 2);

  // CNF 00 Mode 11 0011    
  if (IOP_LED_PIN < 8) {
    bitOffset = IOP_LED_PIN * 4;	// GPIOC->CRH register'�nda C13'e kar��l�k gelen bit konumu
    
    GPIOC->CRL &= ~(0xF << bitOffset);   // GPIOC->CRH<23:20> &= 0000       ..000111100... 11110000111 
    GPIOC->CRL |= (3 << bitOffset);      // GPIOC->CRH<23:20> |= 0011      // *(uint32_t *)0x40011004 = ...
  }
  else {
    bitOffset = (IOP_LED_PIN - 8) * 4;	// GPIOC->CRH register'�nda C13'e kar��l�k gelen bit konumu
    
    GPIOC->CRH &= ~(0xF << bitOffset);   // GPIOC->CRH<23:20> &= 0000       ..000111100... 11110000111 
    GPIOC->CRH |= (3 << bitOffset);      // GPIOC->CRH<23:20> |= 0011      // *(uint32_t *)0x40011004 = ...
  }
  
  // 3) ��k��-0 ��k��-1 durumlar� i�in ODR'de ilgili biti de�i�tir 
  // ODR'de ilgili biti 1 ya da 0 yaparak ��k��-0 / ��k��-1 durumlar�n� sa�l�yoruz
  /*
  GPIOC->ODR |= (1 << IOP_LED_PIN);
  GPIOC->ODR &= ~(1 << IOP_LED_PIN);
  GPIOC->ODR |= (1 << IOP_LED_PIN);
  GPIOC->ODR &= ~(1 << IOP_LED_PIN);	// En son LED yanar konumda kalacak!
  */

  GPIOC->BSRR = (1 << (IOP_LED_PIN + 16));     // DR<13> = 1
  GPIOC->BSRR = (1 << IOP_LED_PIN);     // DR<13> = 1
  GPIOC->BRR = (1 << IOP_LED_PIN);     // DR<13> = 1
  GPIOC->BSRR = (1 << IOP_LED_PIN);     // DR<13> = 1

  while (1) 
  {
    GPIOC->ODR ^= (1 << IOP_LED_PIN);
    
    Delay();
  }
  
  return 0;
}
