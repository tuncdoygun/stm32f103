#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "button.h"

int g_dbMax = 50; // parazit geniþliði.
int _bScan = 0;   // tarama baþlangýç flag deðeri
static BTN_PIN _bts[] = {
  {IOP_BTN_SET, 1, 0, 0}, // current statelerin 1 olmasýnýn sebebi pullup olmasý
  {IOP_BTN_UP, 1, 0, 0},
  {IOP_BTN_DN, 1, 0, 0},  
};

#define N_BUTTONS       (sizeof(_bts) / sizeof(BTN_PIN))

unsigned g_Buttons[N_BUTTONS]; // semafor mekanizmasýnda kullanýlacak (binary / counting)
unsigned g_ButtonsL[N_BUTTONS]; // Long press semaphore

// Her "button timer" tick ile çaðýrýlýr
// Tek bir butonun tarama iþlemini yapar
static void BTN_Scan(int btIdx)
{
  int r; // pin okuma deðeri
  
  r = IO_Read(_bts[btIdx].ioIdx);
  
  if ( r != _bts[btIdx].cState) { // 50 kere ard arda buton durumu ayný okunmalý.Basýlý tutulduðunda cState ile okunan deðer ayný olduðundan bu if'e girmez.
    if (++_bts[btIdx].dbc >= g_dbMax) { 
      // baþarý sayýsýna ulaþýldý: güncel durum deðiþtir.
      _bts[btIdx].cState = r; // durumu okunan deðer yap
      _bts[btIdx].dbc = 0;
      
      if (_bts[btIdx].cState == _bts[btIdx].aState){ // pullup ise aktif state = 0'dýr.
        // Signal mekanizmasý, butona baþarýlý þekilde basýldý.
        // g_Buttons[btIdx] = 1;   // binary semaphore
        ++g_Buttons[btIdx];     // counting semaphore
      }
#ifdef BTN_LONG_PRESS
      else {  
        _bts[btIdx].lstate = 0;
      }
#endif
        
    }
  } else {
    // max baþarý sayýsýna ulaþýlamadan hata geldi.
    _bts[btIdx].dbc = 0;
  }
  
#ifdef BTN_LONG_PRESS
  if (_bts[btIdx].lstate == 0){
    if (_bts[btIdx].cState  == _bts[btIdx].aState)
      if(++_bts[btIdx].acc >= BT_LP_TIME) {
        _bts[btIdx].acc = 0;
        _bts[btIdx].lstate = 1;
        g_ButtonsL[btIdx] = 1; // binary semaphore
      }
  }
#endif
}

void BTN_InitButtons(void)
{
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    IO_Init(_bts[i].ioIdx, IO_MODE_INPUT_PU);
  
  _bScan = 1;
}

void BTN_ScanButtons(void) // Sys_ClockTick içinde kullanýlýyor.
{
  if (!_bScan)
    return;
  
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    BTN_Scan(i);
}