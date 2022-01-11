#ifndef _BUTTON_H
#define _BUTTON_H

#define BT_LP_TIME      3000 // bu tick boyunca aktif kalýrsa.1ms ticktimer olduðundan 3 sn basýlý tutulmasý
#define BTN_LONG_PRESS

typedef struct {
  int ioIdx;    // buttonun baðlý olduðu input pin index
  int cState;   // current state, butonun güncel durumu, belirli periyotlarla okuma yapýlacak,o okumanýn deðeri
  int aState;   // active state, aktif durum, aktifken sýfýr mý aktifken 1 mi
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
void BTN_ScanButtons(void); // tek bir buton taramasýný yapacak.

#endif