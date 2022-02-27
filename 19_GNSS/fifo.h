#ifndef _FIFO_H
#define _FIFO_H

enum { FALSE, TRUE };

typedef struct {
  unsigned char *Buffer;// Verinin saklandýðý bellek
  unsigned char *Head;  // Kuyruk baþý göstericisi
  unsigned char *Tail;  // Kuyruk sonu göstericisi
  int Size;             // Buffer büyüklüðü
  
  unsigned char Full;   // Buffer full flag
  unsigned int  nLines; // Kuyruktaki cümle sayýsý
} FIFO;


void FIFO_Init(FIFO *pFifo, unsigned char *buf, int size);
void FIFO_Clear(FIFO *pFifo);
int FIFO_IsEmpty(FIFO *pFifo);
int FIFO_IsFull(FIFO *pFifo);
int FIFO_SetData(FIFO *pFifo, unsigned char val);
unsigned char FIFO_GetData(FIFO *pFifo);
int FIFO_GetLine(FIFO *pFifo, unsigned char *LineBuf);


#endif
