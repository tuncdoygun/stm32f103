#include <stdio.h>
#include <stdint.h>

#include "io.h"
#include "button.h"

int g_dbMax = 5;

int _bScan = 0;         // Tarama ba�lang�� flag de�eri

static BTN_PIN _bts[] = {
  { IOP_BTN0, 1, 1, 0 },
  { IOP_BTN1, 1, 1, 0 },
};

#define N_BUTTONS       (sizeof(_bts) / sizeof(BTN_PIN))

unsigned g_Buttons[N_BUTTONS];  // Button semaphore (binary / counting)
unsigned g_ButtonsL[N_BUTTONS];  // Button long press semaphore (binary)

// Her "button timer" tick ile �a�r�l�r
// Tek bir butonun tarama i�lemini yapar
static void BTN_Scan(int btIdx)
{
  int r; // pin okuma de�eri
  
  r = IO_Read(_bts[btIdx].ioIdx);
  
  if (r != _bts[btIdx].cState) {
    if (++_bts[btIdx].dbc >= g_dbMax) {
      // ba�ar� say�s�na ula��ld�: durum de�i�tir
      _bts[btIdx].cState = r;
      
      _bts[btIdx].dbc = 0;
      
      if (_bts[btIdx].cState == _bts[btIdx].aState) {
        // Signal !!!
        //g_Buttons[btIdx] = 1;   // binary semaphore
        ++g_Buttons[btIdx];     // counting semaphore
      }
#ifdef BTN_LONG_PRESS
      else {
        _bts[btIdx].lState = 0;
      }
#endif        
    }
  }
  else {
    // max ba�ar� say�s�na ula��lamadan hata geldi
    // sayac� s�f�rl�yoruz
    _bts[btIdx].dbc = 0;
  }
  
#ifdef BTN_LONG_PRESS
  if (_bts[btIdx].lState == 0) {
    if (_bts[btIdx].cState == _bts[btIdx].aState)
      if (++_bts[btIdx].acc >= BT_LPTIME) {
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
    IO_Init(_bts[i].ioIdx, IO_MODE_INPUT_PU);
  
  _bScan = 1;
}

void BTN_ScanButtons(void)
{
  if (!_bScan)
    return;
  
  int i;
  
  for (i = 0; i < N_BUTTONS; ++i)
    BTN_Scan(i);
}


