#ifndef _SI2C_H
#define _SI2C_H

void SI2C_Init(void);
int SI2C_Start(unsigned char ctl);
void SI2C_Stop(void);

int SI2C_Write(unsigned char devAdr, const void *buf, int len);
int SI2C_Read(unsigned char devAdr, void *buf, int len);

int SI2C_WriteA(unsigned char devAdr, unsigned char regAdr, const void *buf, int len);
int SI2C_ReadA(unsigned char devAdr, unsigned char regAdr, void *buf, int len);

#endif