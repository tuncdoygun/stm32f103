#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "button.h"

int g_dbMax = 5; // parazit geni�li�i.
int _bScan = 0;   // tarama ba�lang�� flag de�eri
static BTN_PIN _bts[] = {
  //{ IOP_BTN0, 0, 0, 0 }, // butonu birakinca islem yapacak.
  //{ IOP_BTN1, 0, 0, 0 },
  { IOP_BTN0, 0, 1, 0 }, // butona basinca islem yapacak.standby mod icin.
  { IOP_BTN1, 0, 1, 0 },  
};

#define N_BUTTONS       (sizeof(_bts) / sizeof(BTN_PIN))

unsigned g_Buttons[N_BUTTONS]; // semafor mekanizmas�nda kullan�lacak (binary / counting)
unsigned g_ButtonsL[N_BUTTONS]; // Long press semaphore

// Her "button timer" tick ile �a��r�l�r
// Tek bir butonun tarama i�lemini yapar
static void BTN_Scan(int btIdx)
{
  int r; // pin okuma de�eri
  
  r = IO_Read(_bts[btIdx].ioIdx);
  
  if ( r != _bts[btIdx].cState) { // 50 kere ard arda buton durumu ayn� okunmal�.Bas�l� tutuldu�unda cState ile okunan de�er ayn� oldu�undan bu if'e girmez.
    if (++_bts[btIdx].dbc >= g_dbMax) { 
      // ba�ar� say�s�na ula��ld�: g�ncel durum de�i�tir.
      _bts[btIdx].cState = r; // durumu okunan de�er yap
      _bts[btIdx].dbc = 0;
     
      if (_bts[btIdx].cState == _bts[btIdx].aState){ // pullup ise aktif state = 0'd�r.
        // Signal mekanizmas�, butona ba�ar�l� �ekilde bas�ld�.
        //g_Buttons[btIdx] = 1;   // binary semaphore
        ++g_Buttons[btIdx];     // counting semaphore 
      }
#ifdef BTN_LONG_PRESS
      else { 
       _bts[btIdx].lState = 0;
      }
#endif      
    }
  } else {
    // max ba�ar� say�s�na ula��lamadan hata geldi.
    _bts[btIdx].dbc = 0;
  }
  
#ifdef BTN_LONG_PRESS
  if (_bts[btIdx].lState == 0) {
    if (_bts[btIdx].cState == _bts[btIdx].aState)
      if (++_bts[btIdx].acc >= BT_LP_TIME) {
        _bts[btIdx].acc = 0;
        _bts[btIdx].lState = 1;
        g_ButtonsL[btIdx] = 1; // binary semaphore
      }
  }
#endif
}

void BTN_InitButtons(void)
{
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    IO_Init(_bts[i].ioIdx, IO_MODE_INPUT_PD); // standby mod icin.
    // IO_Init(_bts[i].ioIdx, IO_MODE_INPUT_PU);
  
  _bScan = 1;
}

void BTN_ScanButtons(void) // Sys_ClockTick i�inde kullan�l�yor.
{
  if (!_bScan)
    return;
  
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    BTN_Scan(i);
}
