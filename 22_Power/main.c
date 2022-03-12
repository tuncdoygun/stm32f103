#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
#include "timer.h"
#include "button.h"
#include "exti.h"

void init(void)
{
  // System Clock init
  // Sys_ClockInit(); yorum satiri,sebebi 1 ms'de bir kesme olusmasin.sleep mod efektif kullanilmaz.
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1); // 0 olarak baþlatmak için
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console baþlangýç
  Sys_ConsoleInit();

  // Button baþlangýç
  BTN_InitButtons();
}

// 29.07.2021
// yukardakine göre daha optimize
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

void Task_Print(void)
{
  static unsigned long count;
  
  OLED_SetCursor(1, 0);
  printf("%8lu", ++count);
}

void Task_Button(void)
{
  static unsigned long count;
  
  if (g_Buttons[0]) {
    OLED_SetCursor(0, 0);
    printf("B0:%5lu", ++count);    
    
    g_Buttons[0] = 0;
  }
}

int main_SLEEP()
{
  init();

  //Sys_ClockInit();
  
  OLED_SetFont(FNT_BIG);
  printf("Run");
  
  Timer_Init(TIMER_1, 2000, 36000, 5); // kesme 5 sn
  //Timer_IntConfig(TIMER_1, 2);
  Timer_EvtConfig(TIMER_1);
  Timer_Start(TIMER_1, 1);
  
  SCB->SCR |= SCB_SCR_SEVONPEND; // o interrupt flag biti olustuysa event olusturabilsin.

  while (1)
  {
    Task_LED();
    Task_Print();
    
    OLED_SetCursor(0, 0);
    printf("Stop");
    
    // Sleep mode WFI: wait for interrupt.
    //__WFI(); // islemci bu komuttan sonra sleep moda gececek.5 sn sonra timer1 kesmesi ile uyanacak.
    
    // Sleep mode WFE: wait for event.
    Timer_EvtClear(TIMER_1);
    __WFE();
    
    OLED_SetCursor(0, 0);
    printf("Run ");
  }
}


//A2'ye bir basildiginda kapatacak,bir basildiginda acilacak.Stop moda sokup cikaracak.
void Task_Stop(void)
{
  if (g_Buttons[1]) {
    // Gereksiz kaynaklar kapatýlýr
    IO_Write(IOP_LED, 1);
    // ...
    
    // Stop mode WFI
    OLED_SetCursor(0, 0);
    printf("Stop");
    
    // Event ile stop moddan cikiyor.
    //EXTI_EvtClear(IOP_BTN1);
    //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFE);// regulator acik sekilde stop et:PWR_Regulator_ON
    //
    
    // Interrupt ile stop moddan cikiyor.
    PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);// regulator acik sekilde stop et:PWR_Regulator_ON
    //    
    
    // Normal çalýþma frekansý
    SystemInit();       
    
    OLED_SetCursor(0, 0);
    printf("Run ");
    
    IO_Write(IOP_LED, 0);
    
    g_Buttons[1] = 0;
  }
}

int main_STOP()
{
  init();

  Sys_ClockInit();
  
  OLED_SetFont(FNT_BIG);
  printf("Run");
  IO_Write(IOP_LED, 0);
    
  //EXTI_IntConfig(IOP_BTN1, I_FALLING, 1, 0); // EVENT ile.
  EXTI_IntConfig(IOP_BTN1, I_FALLING, 1, 1); // Interrupt ile.falling edge,buton birakildiginde (1->0) kesme olusacak.
  
  while (1)
  {
    Task_Print();
    
    Task_Stop();
  }
}

void Task_Standby(void)
{
  if (g_Buttons[0]) {
    // Gereksiz kaynaklar kapatýlýr
    // (Dýþ birimler)
    
    // Stop mode WFI
    OLED_SetCursor(0, 0);
    printf("Standby");
    
    PWR_EnterSTANDBYMode();
  }
}

int main()
{
  init();

  Sys_ClockInit();
  
  OLED_SetFont(FNT_BIG);
  printf("Run");

  // Wakeup pin config - rising edge
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // A0 artýk standby'dan uyandirmak icin kullanilacak.
  PWR_WakeUpPinCmd(ENABLE); // RM'ye gore bu mod aktif edildiginde input pulldown'a zorlanir pin.Ve pin 0->1 olunca standby modundan cikar.
                            // o sebepten butonlarý standby modu icin baslangicta input pulldown ve direk basinca aktif olacak sekilde ayarladim.
  
  while (1)
  {
    Task_LED();
    Task_Print();
    //Task_Button();
    Task_Standby();
  }
}
