#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "uart.h"
#include "spi.h"
#include "nRF24.h"
#include "oled.h"

void init(void)
{
  // System Clock init
  Sys_ClockInit();
  
  // I/O portlar� ba�lang��
  Sys_IoInit();
  
  // LED ba�lang��
  IO_Write(IOP_LED, 1); // 0 olarak ba�latmak i�in
  IO_Init(IOP_LED, IO_MODE_OUTPUT);
  
  // Console ba�lang��
  Sys_ConsoleInit();  
  
  SPI_Start();
    
  // init hardware pins
  nrf24_init();
    
  // Channel #2 , payload length: 4
  nrf24_config(2, 4);
}

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
  
  printf("SAYI:%10u\r", ++count);
}

int main()
{
  uint8_t temp;
  uint8_t q = 0;
  uint8_t data_array[4];
  uint8_t tx_address[5] = {0xE7,0xE7,0xE7,0xE7,0xE7};
  uint8_t rx_address[5] = {0xD7,0xD7,0xD7,0xD7,0xD7};
  
  // Ba�lang�� yap�land�rmalar�
  init();

  OLED_SetFont(FNT_SMALL);
  printf("Merhaba nRF!\n");
  
  // Set the device addresses 
  nrf24_tx_address(tx_address);
  nrf24_rx_address(rx_address);  

  //printConfigReg();
  //printStatusReg();
    
  // G�rev �evrimi (Task Loop)
  // Co-Operative Multitasking (Yard�mla�mal� �oklu g�rev)
  while (1)
  {
    Task_LED();  
    
    /* Fill the data buffer */
    data_array[0] = 0x00;
    data_array[1] = 0xAA;
    data_array[2] = 0x55;
    data_array[3] = q++;                                    

    /* Automatically goes to TX mode */
    nrf24_send(data_array);        
      
    /* Wait for transmission to end */
    while(nrf24_isSending());

    /* Make analysis on last tranmission attempt */
    temp = nrf24_lastMessageStatus();

    if(temp == NRF24_TRANSMISSON_OK)                  
        printf("Tranmission  OK\r\n");
    else if(temp == NRF24_MESSAGE_LOST)                  
        printf("Message is lost\r\n");    
        
    /* Retranmission count indicates the tranmission quality */
    temp = nrf24_retransmissionCount();
    printf("Retranmission count: %d\r\n", temp);

    /* Optionally, go back to RX mode ... */
    // nrf24_powerUpRx();
    //printConfigReg();
    //printStatusReg();
    /* Or you might want to power down after TX */
    nrf24_powerDown();            
    
    /* Wait a little ... */
    DelayMs(100);
  }
}


