#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "system.h"
#include "io.h"

volatile int g_bEOC;    // End of conversion flag

void IADC_IoInit(int idx)
{
  IO_Init(idx, IO_MODE_ANALOG);
}

// rank : kanal kacinci sirada olsun.1 - 16. Listeye ekleme yapmaya yarar.
void IADC_Channel(int ch, int rank)
{
  ADC_RegularChannelConfig(ADC1, ch, rank, ADC_SampleTime_239Cycles5);
}

// nConv: Toplam kanal say�s� (listedeki).suanlik 1, daha fazlasi icin DMA kullanmak gerekli.
// cmode: TRUE/FALSE s�rekli mod
// smode: TRUE/FALSE tarama modu
void IADC_Init(int nConv, int cmode, int smode)
{
  // 1) ADC clock setup
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div4); // 18 MHz clock uretilmis olacak. 
  
  ADC_InitTypeDef adcInit;
  
  //ADC_StructInit(&adcInit);// cok structure elemani oldugundan, default degerlerle baslatmak icin.
  
  // 2) �evresel Structure de�erleri
  adcInit.ADC_ContinuousConvMode = cmode ? ENABLE : DISABLE;
  adcInit.ADC_DataAlign = ADC_DataAlign_Right;
  adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  adcInit.ADC_Mode = ADC_Mode_Independent;
  adcInit.ADC_NbrOfChannel = nConv;
  adcInit.ADC_ScanConvMode = smode ? ENABLE : DISABLE;
  
  ADC_Init(ADC1, &adcInit);
  
  // 3) �evresel birimi ba�lat�yoruz
  ADC_Cmd(ADC1, ENABLE);
  
  // 4) Vref ve s�cakl�k sens�r� aktif olacak
  // AIN16: S�cakl�k sens�r� (jonksiyon)
  // AIN17: Internal voltaj reg�lat�r gerilimi
  ADC_TempSensorVrefintCmd(ENABLE);
  
  // 5) ADC kalibrasyon
  // a) Kalibrasyon register reset
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1)) ;
  // b) Kalibrasyonu ba�lat
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1)) ;
}

// Software trigger
// ADC i�lemini yaz�l�msal olarak ba�lat�r
void IADC_Start(void)
{
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// ADC tamamland���nda sonu� register'� de�erine
// geri d�ner
int IADC_Result(void)
{
  return ADC_GetConversionValue(ADC1);
}

// 0 - 4095 arasi sayiya geri doner.
// Ya surekli cevrim yapmak icin IADC_Start cagirilacak ya da continious mod acik olacak.(IADC_Init fonksiyonunda).
// Surekli IADC_Start cagrilmazsa 1 kere cevrim yapar birakir.
int IADC_Convert(int ch)
{
  // 1) Kanal� listeye ekliyoruz
  IADC_Channel(ch, 1);
  
  // 2) AD d�n���m� ba�lat�yoruz
  IADC_Start();
  
  // 3) D�n���m bitene dek bekliyoruz
  while (ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET) ;
  
  // 4) Sonucu okuyup geri d�n�yoruz
  return ADC_GetConversionValue(ADC1);
}

// ADC EOC (End of Conversion) kesmesi yap�land�rmas�
void IADC_IntConfig(void)
{
  // �evresel b�lge ayarlar
  ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
  
  // �ekirdek NVIC ayarlar�
  NVIC_SetPriority(ADC1_2_IRQn, 2);
  NVIC_EnableIRQ(ADC1_2_IRQn);
}

////////////////////////////////////////////////////

void ADC1_2_IRQHandler(void)
{
  if (ADC_GetITStatus(ADC1, ADC_IT_EOC) == SET) {
    g_bEOC = 1;
    
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
  }
}
