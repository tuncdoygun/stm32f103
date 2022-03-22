#ifndef _BUTTON_H
#define _BUTTON_H

#define BTN_LONG_PRESS
#define BT_LPTIME      300

typedef struct {
  int   ioIdx;  // input pin index
  int   cState; // current state
  int   aState; // active state
  int   dbc;    // debounce counter
#ifdef BTN_LONG_PRESS
  int   acc;    // active counter
  int   lState; // long press active 
#endif  
} BTN_PIN;

enum {
  BTN_SET,
  BTN_UP,
  BTN_DN,
};

void BTN_InitButtons(void);
void BTN_ScanButtons(void);

extern unsigned g_Buttons[];
extern unsigned g_ButtonsL[];
#endif