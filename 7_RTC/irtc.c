#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "irtc.h"

#define IRTC_FLAG 0xE3B8 // uydurulmus deger.BKP_DR1'e daha önce yazdýðýmýz deðer,tekrar okuduðumuzda ayný deðeri okuyorsak BKP kapanmamýþ demektir.
#define RTC_PS 32768

volatile int g_Time = 0;
volatile int g_RtcAlarm = 0;
volatile int g_RtcChanged = 0;


// RTC Interrupt Handler
// Farklý 3 kaynaktan ortaya çýkan kesmeleri iþleyecek.
void RTC_IRQHandler(void) // startup dosyasýndaki isimle ayný.Her bir saniye deðiþiminde bu fonksiyona gelir.
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
  if(RTC_GetITStatus(RTC_IT_OW)) {// Get Event Flag. Saat 06:28:15'de taþýyor.
    // Sayacý geniþletme örneði
    // sembolik -> ++BKP_DR2; // Aslýnda bu BKP domainde olmalý.timer RTC'de Vbat ile saydýðýndan taþtýktan sonra da deðer kaybolmamalý.
    RTC_ClearITPendingBit(RTC_IT_OW); // temizlenmeli - flag = 0
  }   
}

// Saniye sayacý olarak ref zamanýndan bu yana 
// geçen süreye geri döner.
uint32_t IRTC_GetTime(void)
{
  return RTC_GetCounter();
}

void IRTC_SetTime(uint32_t tmVal)
{
  // RTC'de süren iþlem varsa bekle
  RTC_WaitForLastTask();  
  
  RTC_SetCounter(tmVal);
  
  // RTC'de süren iþlem varsa bekle
  RTC_WaitForLastTask();    
}

void IRTC_Init(void)
{
  // PWR ve BKP birimleri saat isareti
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
  
  // Backup registerlarina erisimi aciyoruz
  PWR_BackupAccessCmd(ENABLE);
  
  if(BKP_ReadBackupRegister(BKP_DR1) != IRTC_FLAG) { // RTC ilk kez baþlatýlýyor.(BKP domain power on reset)
    // Backup Domain resetlenmis
    BKP_DeInit();
    
    // LSE'yi çalýþtýrýyoruz.
    RCC_LSEConfig(RCC_LSE_ON);
    
    // LSE çalýþana kadar bekle
    while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
    
    // RTC clock kaynaðý = LSE olsun
    RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
    // RTC clock aktif
    RCC_RTCCLKCmd(ENABLE);
    
    // RTC registerlarýna eriþim için senkronlamak gerekiyor.
    RTC_WaitForSynchro();
    
    // RTC'de süren iþlem varsa bekle
    RTC_WaitForLastTask();
    
    // RTC prescaler ayarý (LSE periyot register)
    RTC_SetPrescaler(RTC_PS - 1);
    
    // Ýþlem tamamlandý.
    BKP_WriteBackupRegister(BKP_DR1, IRTC_FLAG); // burda ilk RTC baþlangýcýnda rastgele IRTC_FLAG deðeri atadýk.Daha sonra RTC'nin çalýþýp çalýþmadýðýný
    // veya ilk defa çalýþtýrýlacaðýný anlamak için bu registerý okuyup karar vericez.
    // Normalde RTC ve BKP registerlarý baðýmsýz.Sadece DR1 registerýný ödünç alýp okuma/yazma yaptýk.
    // Çünkü Vbat baðlýysa bkp registerlarý kapanmaz.
  } else { // RTC daha önce baþlatýlmýþ.
    // RTC registerlarýna eriþim için senkronlamak gerekir.
    RTC_WaitForSynchro();
  }
}

void IRTC_IntConfig(void)
{
  // RTC second 
  // RTC Periodic interrupt
  // sayacýn her bir artýþýnda kesme alabiliyoruz.
  
  // 1 - Peripheral (çevresel)
  // 2 - Core (NVIC)
  

  RTC_ClearITPendingBit(RTC_IT_SEC); // 0 - false interrupt önlemi
  RTC_ITConfig(RTC_IT_SEC, ENABLE);  // 1 - rtc saniye kesmesi enable

  RTC_ClearITPendingBit(RTC_IT_ALR); // 0 - false interrupt önlemi
  RTC_ITConfig(RTC_IT_ALR, ENABLE);  // 1 - rtc alarm kesmesi enable

  RTC_ClearITPendingBit(RTC_IT_OW); // 0 - false interrupt önlemi
  RTC_ITConfig(RTC_IT_OW, ENABLE);  // 1 - rtc overflow kesmesi enable
  
  // hepsi NVIC kullandýðýndan 1 kere yeterli.
  NVIC_SetPriority(RTC_IRQn, 3);     // 2a - RTC kesme önceliði = 3. en küçük numara en büyük öncelik
  NVIC_EnableIRQ(RTC_IRQn);          // 2b - RTC saniye kesmesini aktif ettik.Handler fonksiyonu yazýlmalý.startup dosyasýndaki fonksiyonla ayný olmalý fonksiyon ismi
  
  // RTC Alarm
  
  // RTC Overflow
  
}

void IRTC_SetAlarm(uint32_t alrVal)
{
  RTC_SetAlarm(alrVal);
}
