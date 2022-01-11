#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "irtc.h"

#define IRTC_FLAG 0xE3B8 // uydurulmus deger.BKP_DR1'e daha �nce yazd���m�z de�er,tekrar okudu�umuzda ayn� de�eri okuyorsak BKP kapanmam�� demektir.
#define RTC_PS 32768

volatile int g_Time = 0;
volatile int g_RtcAlarm = 0;
volatile int g_RtcChanged = 0;


// RTC Interrupt Handler
// Farkl� 3 kaynaktan ortaya ��kan kesmeleri i�leyecek.
void RTC_IRQHandler(void) // startup dosyas�ndaki isimle ayn�.Her bir saniye de�i�iminde bu fonksiyona gelir.
{
  // RTC Second
  if(RTC_GetITStatus(RTC_IT_SEC)) {// Get Event Flag
    g_RtcChanged = 1;
  
    RTC_ClearITPendingBit(RTC_IT_SEC); // temizlenmeli - flag = 0
  }
  
  // RTC Alarm
  if(RTC_GetITStatus(RTC_IT_ALR)) {// Get Event Flag
    g_RtcAlarm = 1;
    RTC_ClearITPendingBit(RTC_IT_ALR); // temizlenmeli - flag = 0
  }  
  
  // RTC overflow
  if(RTC_GetITStatus(RTC_IT_OW)) {// Get Event Flag. Saat 06:28:15'de ta��yor.
    // Sayac� geni�letme �rne�i
    // sembolik -> ++BKP_DR2; // Asl�nda bu BKP domainde olmal�.timer RTC'de Vbat ile sayd���ndan ta�t�ktan sonra da de�er kaybolmamal�.
    RTC_ClearITPendingBit(RTC_IT_OW); // temizlenmeli - flag = 0
  }   
}

// Saniye sayac� olarak ref zaman�ndan bu yana 
// ge�en s�reye geri d�ner.
uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
  // RTC'de s�ren i�lem varsa bekle
  RTC_WaitForLastTask();  
  
  RTC_SetCounter(tmVal);
  
  // RTC'de s�ren i�lem varsa bekle
  RTC_WaitForLastTask();    
}

void IRTC_Init(void)
{
  // PWR ve BKP birimleri saat isareti
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  // Backup registerlarina erisimi aciyoruz
  PWR_BackupAccessCmd(ENABLE);
  
  if(BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) { // RTC ilk kez ba�lat�l�yor.(BKP domain power on reset)
    // Backup Domain resetlenmis
    BKP_DeInit();
    
    // LSE'yi �al��t�r�yoruz.
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE �al��ana kadar bekle
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    // RTC clock kayna�� = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock aktif
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC registerlar�na eri�im i�in senkronlamak gerekiyor.
    RTC_WaitForSynchro();
    
    // RTC'de s�ren i�lem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescaler ayar� (LSE periyot register)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // ��lem tamamland�.
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG); // burda ilk RTC ba�lang�c�nda rastgele IRTC_FLAG de�eri atad�k.Daha sonra RTC'nin �al���p �al��mad���n�
    // veya ilk defa �al��t�r�laca��n� anlamak i�in bu register� okuyup karar vericez.
    // Normalde RTC ve BKP registerlar� ba��ms�z.Sadece DR1 register�n� �d�n� al�p okuma/yazma yapt�k.
    // ��nk� Vbat ba�l�ysa bkp registerlar� kapanmaz.
  } else { // RTC daha �nce ba�lat�lm��.
    // RTC registerlar�na eri�im i�in senkronlamak gerekir.
    RTC_WaitForSynchro();
  }
}

void IRTC_IntConfig(void)
{
  // RTC second 
  // RTC Periodic interrupt
  // sayac�n her bir art���nda kesme alabiliyoruz.
  
  // 1 - Peripheral (�evresel)
  // 2 - Core (NVIC)
  

  RTC_ClearITPendingBit(RTC_IT_SEC); // 0 - false interrupt �nlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE);  // 1 - rtc saniye kesmesi enable

  RTC_ClearITPendingBit(RTC_IT_ALR); // 0 - false interrupt �nlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE);  // 1 - rtc alarm kesmesi enable

  RTC_ClearITPendingBit(RTC_IT_OW); // 0 - false interrupt �nlemi
  RTC_ITConfig(RTC_IT_OW, ENABLE);  // 1 - rtc overflow kesmesi enable
  
  // hepsi NVIC kulland���ndan 1 kere yeterli.
  NVIC_SetPriority(RTC_IRQn, 3);     // 2a - RTC kesme �nceli�i = 3. en k���k numara en b�y�k �ncelik
  NVIC_EnableIRQ(RTC_IRQn);          // 2b - RTC saniye kesmesini aktif ettik.Handler fonksiyonu yaz�lmal�.startup dosyas�ndaki fonksiyonla ayn� olmal� fonksiyon ismi
  
  // RTC Alarm
  
  // RTC Overflow
  
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal);
}
