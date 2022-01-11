#ifndef _BUTTON_H
#define _BUTTON_H

#define BT_LP_TIME      3000 // bu tick boyunca aktif kal�rsa.1ms ticktimer oldu�undan 3 sn bas�l� tutulmas�
#define BTN_LONG_PRESS

typedef struct {
  int ioIdx;    // buttonun ba�l� oldu�u input pin index
  int cState;   // current state, butonun g�ncel durumu, belirli periyotlarla okuma yap�lacak,o okuman�n de�eri
  int aState;   // active state, aktif durum, aktifken s�f�r m� aktifken 1 mi
  int dbc;      // debounce counter
#ifdef BTN_LONG_PRESS
  int acc;      // active counter
  int lstate;   // long press durum
#endif
} BTN_PIN;

enum {
  BTN_SET,
  BTN_UP,
  BTN_DN,
};

extern unsigned g_Buttons[];
extern unsigned g_ButtonsL[];

void BTN_InitButtons(void);
void BTN_ScanButtons(void); // tek bir buton taramas�n� yapacak.

#endif