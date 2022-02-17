#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "oled.h"
#include "button.h"
#include "1-Wire.h"
#include "crc8.h"

#define SYS_CLOCK_FREQ  72000000

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlarý baþlangýç
  Sys_IoInit();
  
  // LED baþlangýç
  IO_Write(IOP_LED, 1);
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console baþlangýç
  Sys_ConsoleInit();
  
  // Button baþlangýç
  BTN_InitButtons();
  
  // 1-Wire Bus baþlangýç
  OW_Init();  
}

void Task_LED(void)
{
  static enum {
    I_LED_OFF,
    S_LED_OFF,
    I_LED_ON,
    S_LED_ON,
  } state = I_LED_OFF;
  
  static clock_t t0;    // Duruma ilk geçiþ saati
  clock_t t1;           // Güncel saat deðeri
  
  t1 = clock();
  
  switch (state) {
  case I_LED_OFF:
      t0 = t1;      
      IO_Write(IOP_LED, 1);     // LED off
      state = S_LED_OFF;
      //break;    
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10) 
      state = I_LED_ON;
    break;
  
  case I_LED_ON:
    t0 = t1;
    IO_Write(IOP_LED, 0);     // LED On
    state = S_LED_ON;
    //break;    
  case S_LED_ON:
    if (t1 >= t0 + CLOCKS_PER_SEC / 10) 
      state = I_LED_OFF;
    break;
  }  
}

void Task_Print(void)
{
  OLED_SetCursor(1, 0);
  
  printf("Clock:%10lu", clock());
}

void Task_Button(void)
{
  static unsigned long nPressed = 0;
  
  if (g_Buttons[0]) {
    --g_Buttons[0];
    
    ++nPressed;
    
    OLED_SetCursor(2, 0);
    printf("SAYI:%6lu", nPressed);
  }
}   

// DS18B20 sýcaklýðýný okur ve display'de gösterir
// Bloke çalýþýr
void Task_ThermoB(void)
{
  int temp;
  
  OLED_SetCursor(3, 0);
  
  if (!DS_ReadTemp(&temp))
    printf("ERROR           ");
  else
    printf("Temp:% 6.1f", (float)temp / 16);   
}

// State machine
void Task_ThermoS(void)
{
  int temp;
  
  temp = Task_Thermo();
  if (temp != TEMP_INVALID) {
    OLED_SetCursor(3, 0);  
    printf("Temp:% 6.1f", (float)temp / 16);   
  }
}

int main()
{
  // Baþlangýç yapýlandýrmalarý
  init();

  //unsigned char dsId[8];
  //int i;
  
  OLED_SetFont(FNT_LARGE);
  printf("Hello, world!\n");
  
  /*
  OLED_SetCursor(3, 0);
  if (DS_ReadROM(dsId)) {
    for (i = 7; i >= 0; --i)
      printf("%02X", dsId[i]);
  }
  else {
    printf("Hata: DS yok!");
  }  
  */
  
  // Görev çevrimi (Task Loop)
  // Co-Operative Multitasking (Yardýmlaþmalý çoklu görev) 
  while (1)
  {
    Task_LED();
    Task_Print();
    Task_Button();
    
    /*
    if(OW_Reset()){
    OLED_SetCursor(3, 0);
    printf("OW_Reset: 1");
    }
    */
 
    //Task_ThermoB();
    Task_ThermoS();
    DelayUs(1000); // OW_Reset gorevini duzgun bir sekilde yerine getiremiyor.
                   // 1 ms kadar bir gecikme istiyor olabilir.Ya da ben sensoru ilk basta ters bagladigimdan problem olusmus olabilir.
  }
  
  //return 0;
}


