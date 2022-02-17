#ifndef _1_WIRE_H
#define _1_WIRE_H

#define TEMP_INVALID    (-32768)

void OW_Init(void);
int OW_Reset(void);

int DS_ReadROM(unsigned char *buf);
int DS_ReadTemp(int *pTemp);
int Task_Thermo(void);

#endif