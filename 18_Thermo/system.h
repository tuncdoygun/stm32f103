#ifndef _SYSTEM_H
#define _SYSTEM_H


#define CLOCKS_PER_SEC   1000

typedef unsigned long   clock_t;

clock_t clock(void);

void Sys_ClockInit(void);
void Sys_IoInit(void);
void Sys_ConsoleInit(void);

void Sys_EnterCritical(void);
void Sys_ExitCritical(void);


void __delay(unsigned long);
             
#define CLOCK_FREQ      72000000
#define INSTR_FREQ      ((CLOCK_FREQ) * 8 / 9)
#define US_CYCLES       ((INSTR_FREQ) / 1000000)
#define MS_CYCLES       ((INSTR_FREQ) / 1000)

#define DelayUs(us)     __delay((us) * US_CYCLES)
#define DelayMs(ms)     __delay((ms) * MS_CYCLES)
#endif
