#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "lcdm.h"
#include "uart.h"
#include "button.h"
#include "irtc.h"

// RTC setup timeout de�eri
#define RTC_TMOUT       30000

enum { IRTC_WORK, IRTC_SETUP };
int g_RtcMode = IRTC_WORK;

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1); // 0 olarak ba�latmak i�in
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  //LCDM Ba�lang��
  //LCD_Init();
  
  // Konsol Ba�lang��
  Sys_ConsoleInit();
  
  // Button ba�lang��
  BTN_InitButtons();
  
  // RTC Baslangic
  IRTC_Init();
  IRTC_IntConfig();
}

//int c; // debug modda global de�i�kenlerin de�eri g�r�lebiliyor.IO_Read() i�in.
// 27.07.2021
/*void Task_LED(void)
{
  static enum {
    S_LED_OFF,
    S_LED_ON,
  } state = S_LED_OFF;
   
  static clock_t t0, t1; // t0 duruma ilk ge�i� saati, t1 g�ncel saat
  
  t1 = clock(); // bu fonksiyona girdi�inde o andaki saat
  
  switch (state){
  case S_LED_OFF:
    if (t1 >= t0 + 9 * CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      IO_Write(IOP_LED, 0);
      
      t0 = t1;
      state = S_LED_ON;
    }
    break;
  
  case S_LED_ON:
    if (t1 >= t0 +  CLOCKS_PER_SEC / 10){ // 9/10 saniye ge�mi� demek
      IO_Write(IOP_LED, 1);
      
      t0 = t1;
      state = S_LED_OFF;
    }    
    break;
  }
}*/
//

// 29.07.2021
// yukardakine g�re daha optimize
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

void Task_Print(void)
{
  static unsigned count;
  
  printf("SAYI:%u\r", ++count);
  UART_printf("Count: %10lu\r", count);
}

void Task_Button(void)
{
  static unsigned count = 0;
  
  if (g_Buttons[BTN_SET]){ // BTN_SET'ten i�aret geldi
    // SET i�le
    UART_printf("SET (%u)\n", ++count);
    
    // g_Buttons[BTN_SET] = 0; binary semaphore
    --g_Buttons[BTN_SET]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_Buttons[BTN_UP]){ // BTN_UP'ten i�aret geldi
    // SET i�le
    UART_printf("UP (%u)\n", ++count);
    
    //g_Buttons[BTN_UP] = 0;
    --g_Buttons[BTN_UP]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_Buttons[BTN_DN]){ // BTN_UP'ten i�aret geldi
    // DN i�le
    UART_printf("DN (%u)\n", ++count);
    
    //g_Buttons[BTN_DN] = 0;
    --g_Buttons[BTN_DN]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  //////////////////////////////////////
#ifdef BTN_LONG_PRESS
  if (g_ButtonsL[BTN_SET]){
    // SET i�le
    UART_printf("SET_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_SET] = 0; // binary semaphore
    //--g_Buttons[BTN_SET]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_ButtonsL[BTN_UP]){ // BTN_UP'ten i�aret geldi
    // SET i�le
    UART_printf("UP_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_UP] = 0;
    // --g_Buttons[BTN_UP]; // s�f�ra gelene kadar hep i�leyecek
  }
  
  if (g_ButtonsL[BTN_DN]){ // BTN_UP'ten i�aret geldi
    // DN i�le
    UART_printf("DN_LONG (%u)\n", ++count);
    
    g_ButtonsL[BTN_DN] = 0;
    // --g_Buttons[BTN_DN]; // s�f�ra gelene kadar hep i�leyecek
  }
#endif
}

// mevcut zamandan belli bir saniye sonra alarm
void SetAlarm(uint32_t nSeconds)
{
  uint32_t tm1;
  
  tm1 = IRTC_GetTime();
  tm1 += nSeconds;
  IRTC_SetAlarm(tm1);
}

void Task_Time(void)
{
  if ((g_RtcMode != IRTC_SETUP) && g_RtcChanged){ // 01.09
    printf("\nTime: %10lu", ++g_Time);
    g_RtcChanged = 0;
  }
  // UART_printf("RTC time: %10lu\r", IRTC_GetTime()); // 31.08
}

void Task_Alarm(void)
{
  static unsigned nAlarms;
  
  if ((g_RtcMode != IRTC_SETUP) && g_RtcAlarm){ // 01.09
    LCD_SetCursor(0x0A);
    printf("(%u)", ++nAlarms);
	
    SetAlarm(10); // her 10 saniye sonras�na alarm
    g_RtcAlarm = 0;
  }   
}

void DisplayTime(uint32_t tmVal)
{
  int hour, min, sec;
  
  hour = (tmVal / 3600) % 24;
  min = (tmVal / 60) % 60;
  sec = tmVal % 60;
  
  //UART_printf("\r%02d:%02d:%02d", // \r ilk sat�r�n ba��ndan ba�lamak i�in.
  //       hour, min, sec);
  
  printf("\r%02d:%02d:%02d", // \r ilk sat�r�n ba��ndan ba�lamak i�in.
         hour, min, sec);  
}

void Task_RTC()
{
  static enum {
    I_WORK, 
    S_WORK,
    I_SETHOUR,
    S_SETHOUR,
    I_SETMIN,
    S_SETMIN,
    S_INCHOUR,
    S_DECHOUR,
    S_INCMIN,
    S_DECMIN,
    S_UPDATE,
    S_CANCEL,
  } state = I_WORK;
  
  static clock_t t0, t1; // t1 g�ncel zaman, t0 en son kal�nan zaman
  static uint32_t tm0, tm1; // time degerleri.
  static int hour, min, sec;
  
  t1 = clock(); // g�ncel saat de�eri
  
  switch (state) {
  case I_WORK:
    g_RtcMode = IRTC_WORK;
    LCD_DisplayOn(LCD_ON);
    state = S_WORK;
    // break; // bunu kald�rd�k,direk S_WORK'e ge�ecek zaten.
  
  case S_WORK:
    tm1 = IRTC_GetTime();
    if(tm1 != tm0) { // zaman de�i�mediyse bo�una yazd�rmamak i�in.
      DisplayTime(tm1);
      tm0 = tm1;
    }
    
    if(g_ButtonsL[BTN_SET]){
      hour = (tm1 / 3600) % 24;
      min = (tm1 / 60) % 60;      
      g_ButtonsL[BTN_SET] = 0;
      
      // clear false signals
      g_Buttons[BTN_SET] = 0; // butona k�sa bas�lma semaphore'u da set edilece�inden.clear false signal      
      g_Buttons[BTN_UP] = 0;
      g_Buttons[BTN_DN] = 0;
      state = I_SETHOUR;
    }
    break;
  
  case I_SETHOUR:
    g_RtcMode = IRTC_SETUP;
    
    LCD_DisplayOn(LCD_ON | LCD_BLINK);

        
    DisplayTime(hour * 3600 + min * 60);
    LCD_SetCursor(1);    
    
    t0 = t1; // 30 sn boyunca hi� i�lem yap�lmazsa timeour,t
    state = S_SETHOUR;
    break;
  
  case S_SETHOUR:
    if (t1 - t0 >= RTC_TMOUT) {
      state = S_CANCEL;
    }
    
    if(g_Buttons[BTN_SET]){
      g_Buttons[BTN_SET] = 0; // butona k�sa bas�lma semaphore'u da set edilece�inden.
      state = I_SETMIN;
    } else if(g_Buttons[BTN_UP]){
      g_Buttons[BTN_UP] = 0;
      state = S_INCHOUR;
    } else if(g_Buttons[BTN_DN]) {
      g_Buttons[BTN_DN] = 0;
      state = S_DECHOUR;
    }
    break;
  
  case I_SETMIN:
    DisplayTime(hour * 3600 + min * 60);
    LCD_SetCursor(4);
       
    t0 = t1;
    state = S_SETMIN;
    break;
  
  case S_SETMIN:
    if (t1 - t0 >= RTC_TMOUT) {
      state = S_CANCEL;
    }    
    
    if(g_Buttons[BTN_SET]){
      g_Buttons[BTN_SET] = 0; // butona k�sa bas�lma semaphore'u da set edilece�inden.
      state = S_UPDATE;
    } else if(g_Buttons[BTN_UP]){
      g_Buttons[BTN_UP] = 0;
      state = S_INCMIN;
    } else if(g_Buttons[BTN_DN]) {
      g_Buttons[BTN_DN] = 0;
      state = S_DECMIN;
    }  
    break;
  
  case S_INCHOUR:
    if(++hour >= 24)
      hour = 0;
    
    state = I_SETHOUR;
    break;
  
  case S_DECHOUR:
    if(--hour <= 0)
      hour = 23;
    
    state = I_SETHOUR;    
    break;
 
  case S_INCMIN:
    if(++min >= 60)
      min = 0;
    
    state = I_SETMIN;    
    break;
  
  case S_DECMIN:
    if(--min <= 0)
      min = 59;
    
    state = I_SETMIN;   
    break;
  
  case S_UPDATE:
    IRTC_SetTime(hour * 3600 + min * 60);
    state = I_WORK;
    break;
  
  case S_CANCEL:  
    state = I_WORK;
    break;
  }
}

int main()
{
  // Ba�lang�� yap�land�rmalar�
  init();
  
  IRTC_SetTime(22 * 3600 + 39 * 60 + 17); // 31.08 22saat 39 dakika 17 saniye
  // IRTC_SetTime(0xFFFFFFF0); // 01.09 overflow i�in test.32 bitlik saya� oldu�undan,16 saniye sonra ta�acak,0 -> F olunca.
  
  SetAlarm(10); // alarmin ilk set edildi�i yer.
  
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev)
  while (1)
  {
    Task_LED(); //di�er tasklar�n do�ru �al���p �al��mad���n�,aksama olup olmad���n� anlamak i�in kullan�labilir.Ak�� devam ediyorsa LED d�zg�n yan�p s�n�yordur.
    //Task_Print();
    //Task_Button();
    Task_Time();
    Task_Alarm();
    // DisplayTime(IRTC_GetTime());
    Task_RTC();
  }
  //return 0;
}


