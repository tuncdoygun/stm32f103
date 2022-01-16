#ifndef _KEYBOARD_H
#define _KEYBOARD_H

#define NROWS   4
#define NCOLS   4

#define KB_INVALIDKEY   (-1)

extern volatile int g_Key;
extern int g_maxDBC;

void KB_Init(void);
void KB_Scan(void);

int _kbhit(void);
int _getch(void);

#endif
