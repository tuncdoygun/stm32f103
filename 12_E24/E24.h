#ifndef _E24_H
#define _E24_H

void E24_Config(int adSize, int pgSize);
int E24_WritePage(int chipAdr, unsigned long offset, const void *buffer, int size);
int E24_Read(int chipAdr, unsigned long offset, void *buffer, int size);

#endif

