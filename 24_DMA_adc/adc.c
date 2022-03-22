#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "system.h"
#include "io.h"
#include "timer.h"

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

// nConv: Toplam kanal sayýsý (listedeki)
// nConv: Listedeki eleman sayýsý (kaç kanal)
// chList: Kanal no listesi
// rsList: Result (sonuç) listesi
// period: örnekleme periyodu (ms)
void IADC_Init_DMA(int nConv, uint8_t *chList, volatile uint16_t *rsList, int period)
{
  int i;
    
  ////////////////////////////////////////////////////
  // Trigger (tetik) ayarlarý
  PWM_InitT4CC4(period);

  /////////////////////////////////////////////////////////////////////
  // DMA Ayarlarý
  DMA_InitTypeDef dmaInit;
  
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  
  DMA_DeInit(DMA1_Channel1);
  
  DMA_StructInit(&dmaInit);
  
  // Genel ayarlar
  dmaInit.DMA_BufferSize = nConv;
  dmaInit.DMA_DIR = DMA_DIR_PeripheralSRC;
  dmaInit.DMA_Priority = DMA_Priority_High;
  dmaInit.DMA_Mode = DMA_Mode_Circular;
  dmaInit.DMA_M2M = DISABLE;
  
  // Kaynak ayarlarý
  dmaInit.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  dmaInit.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  // 16-bit, cunku adc 12 bit cevrim yapiyor.
  dmaInit.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  
  // Hedef ayarlarý
  dmaInit.DMA_MemoryBaseAddr = (uint32_t)rsList;
  dmaInit.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  dmaInit.DMA_MemoryInc = DMA_MemoryInc_Enable;  
  
  DMA_Init(DMA1_Channel1, &dmaInit);
  DMA_Cmd(DMA1_Channel1, ENABLE);
    
  /////////////////////////////////////////////////////////////////////
  
  ADC_InitTypeDef adcInit;
  
  //ADC_StructInit(&adcInit);
  // 1) ADC clock setup
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
  RCC_ADCCLKConfig(RCC_PCLK2_Div8);
    
  // 2) Çevresel Structure deðerleri
  ADC_StructInit(&adcInit);
  adcInit.ADC_ContinuousConvMode = DISABLE;// timer ile tetikleyip istedigimiz zaman cevrim yapacagimizdan disable
  adcInit.ADC_DataAlign = ADC_DataAlign_Right;
  adcInit.ADC_ExternalTrigConv = ADC_ExternalTrigConv_T4_CC4;
  adcInit.ADC_Mode = ADC_Mode_Independent;
  adcInit.ADC_NbrOfChannel = nConv;
  adcInit.ADC_ScanConvMode = ENABLE;
  
  ADC_Init(ADC1, &adcInit);
  
  ADC_ExternalTrigConvCmd(ADC1, ENABLE);
  
  // 3) Çevresel birimi baþlatýyoruz
  ADC_Cmd(ADC1, ENABLE);
  
  // 4) Vref ve sýcaklýk sensörü aktif olacak
  // AIN16: Sýcaklýk sensörü (jonksiyon)
  // AIN17: Internal voltaj regülatör gerilimi
  ADC_TempSensorVrefintCmd(ENABLE);
  
  // 5) ADC kalibrasyon
  // a) Kalibrasyon register reset
  ADC_ResetCalibration(ADC1);
  while (ADC_GetResetCalibrationStatus(ADC1)) ;
  // b) Kalibrasyonu baþlat
  ADC_StartCalibration(ADC1);
  while (ADC_GetCalibrationStatus(ADC1)) ;
  
  ADC_DMACmd(ADC1, ENABLE);
  
  ////////////////////////////////////////////////////
  // Kanal listesi
  for (i = 0; i < nConv; ++i)
    IADC_Channel(chList[i], i + 1);
  
  ////////////////////////////////////////////////////
  // Kesme ayarlarý
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE); // sadece DMA kesmesi olacak,adc degil
  DMA_ClearITPendingBit(DMA1_IT_TC1);
  
  NVIC_SetPriority(DMA1_Channel1_IRQn, 3);
  NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

//////////////////////////////////////////////////////////

void DMA1_Channel1_IRQHandler(void)
{
  if (DMA_GetITStatus(DMA1_IT_TC1) == SET) {
    g_bEOC = 1;
    
    DMA_ClearITPendingBit(DMA1_IT_TC1);
  }
}
