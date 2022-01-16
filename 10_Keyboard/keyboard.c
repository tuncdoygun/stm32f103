#include <stdio.h>
#include <stdint.h>

#include "system.h"
#include "io.h"
#include "fifo.h"
#include "keyboard.h"

#define SZ_KEYBUF       16

volatile int g_Key = KB_INVALIDKEY;
int g_maxDBC = 5;

FIFO_VAR        _keyBuf[SZ_KEYBUF];
FIFO   _kbFifo;

static volatile int _bInit = 0;

static int _rows[NROWS] = {
  IOP_KEY_ROW0, IOP_KEY_ROW1, IOP_KEY_ROW2, IOP_KEY_ROW3, 
};

static int _cols[NCOLS] = {
  IOP_KEY_COL0, IOP_KEY_COL1, IOP_KEY_COL2, IOP_KEY_COL3, 
};

char g_chTab[NROWS * NCOLS] = "0123456789ABCDEF";

////////////////////////////////////////////////////////////////////////////////
// MATRIX KEYBOARD KERNEL

void KB_Init(void)
{
  int i;
  
  FIFO_Init(&_kbFifo, _keyBuf, SZ_KEYBUF);
  
  // Bütün satýrlar input + pull-up
  for (i = 0; i < NROWS; ++i)
    IO_Init(_rows[i], IO_MODE_INPUT_PU);
  
  // Bütün sütunlar çýkýþ-0
  for (i = 0; i < NCOLS; ++i) {
    IO_Write(_cols[i], 0);
    IO_Init(_cols[i], IO_MODE_OUTPUT);
  }
  
  _bInit = 1;
}

static int KB_ScanRows(void)
{
  int i;
  
  for (i = 0; i < NROWS; ++i)
    if (!IO_Read(_rows[i]))
        break;
        
  return i;
}

void KB_Scan(void)
{
  int i, j, k;
  static int dbCount = 0;
  
  if (!_bInit)
    return;
  
  i = KB_ScanRows();
  
  // Daha önce basýlýmýydý?
  if (g_Key == KB_INVALIDKEY) {
    // Hayýr g_Key: -1 (KB_INVALIDKEY)
    for (j = 0; j < NCOLS; ++j) {
      // sadece _cols[j] = 0 olacak, diðerleri 1
      for (k = 0; k < NCOLS; ++k)
        IO_Write(_cols[k], (k == j) ? 0 : 1);
      
      DelayUs(1);  // pin degisikliginde hemen okuma yapildiginda pin direk konum degistirmemis olabilir.
      
      i = KB_ScanRows();
      
      if (i < NROWS) {
        // satýr ve sütun belirlendi
        // satýr: i, sütun: j
        // scan code (g_Key) = row * NCOLS + col        
        g_Key = i * NCOLS + j;
        // tarama kodunu keyboard buffer'a yüklüyoruz
        FIFO_SetData(&_kbFifo, g_Key);
        break;
      }
    }
    
    // Debounce görevi ve bir sonraki tuþun
    // algýlanmasý için tüm sütunlarý 0 yapýyoruz
    for (k = 0; k < NCOLS; ++k)
      IO_Write(_cols[k], 0);    
  }
  else {
    // Evet g_Key = tarama kodu 0..(NROWS-1)
    // Görevimiz debouncing ile tuþun býrakýldýðýndan
    // emin olmak
    if (i >= NROWS) {
      // Tuþ býrakýldý ya da sýçrama
      if (++dbCount >= g_maxDBC) {
        // Tuþ(lar) býrakýldý
        g_Key = KB_INVALIDKEY;
        dbCount = 0;
      }
    }
    else {
      // Sýçrama
      dbCount = 0;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// MATRIX KEYBOARD API

// Keyboard buffer statüsüne geri döner
// 0: Karakter yok, farklýysa karakter var
int _kbhit(void)
{
  return !FIFO_IsEmpty(&_kbFifo);
}

// Matrix keyboard'dan karakter okur
// Buffer boþsa herhangi bir tuþa
// basýlana kadar bloke bekler
int _getch(void)
{
  char ch;
  FIFO_VAR scode;
  
  //while (!_kbhit()) ;
  
  scode = FIFO_GetData(&_kbFifo);
  ch = g_chTab[scode];
  
  return ch;
}
