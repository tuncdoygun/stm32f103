#include <stdio.h>
#include <stdint.h>

#include "fifo.h"

// FIFO yapýsýnýn kuyruk (buffer) alanýný sýfýrlar
void FIFO_Clear(FIFO *pFifo)
{
  pFifo->Head = pFifo->Tail = pFifo->Buffer;
  
  pFifo->nLines = 0;
  pFifo->Full = FALSE;
}

// FIFO yapýsýný baþlatýr
void FIFO_Init(FIFO *pFifo, unsigned char *buf, int size)
{
  pFifo->Buffer = buf;
  pFifo->Size = size;

  FIFO_Clear(pFifo);
}

// FIFO yapýsýnda veri yoksa (boþsa) TRUE 
// veri varsa FALSE
int FIFO_IsEmpty(FIFO *pFifo)
{
  return (pFifo->Head == pFifo->Tail);
}

// FIFO yapýsý tümüyle doluysa TRUE 
// deðilse FALSE
int FIFO_IsFull(FIFO *pFifo)
{
  return pFifo->Full;
}

// FIFO yapýsýna veri ekler
// Ekleme baþarýlýysa TRUE döner
// Aksi durumda (full) FALSE döner
int FIFO_SetData(FIFO *pFifo, unsigned char val)
{
  unsigned char *ptr = pFifo->Tail;
  
  if (++ptr == pFifo->Buffer + pFifo->Size) 
    ptr = pFifo->Buffer;
  
  if (ptr == pFifo->Head) {
    // Buffer full
    pFifo->Full = TRUE;
    return FALSE;
  }
  
  *pFifo->Tail = val; // veri yüklendi
  pFifo->Tail = ptr;
  
  if (val == '\n')
    ++pFifo->nLines;
  
  return TRUE;
}

// FIFO yapýsýndan veri çeker
// Bloke çalýþýr (çekilecek veri yoksa)
unsigned char FIFO_GetData(FIFO *pFifo)
{
  unsigned char val;
  
  while (FIFO_IsEmpty(pFifo)) ;
  
  // FIFO da çekilecek veri var
  val = *pFifo->Head;
  
  // Head göstericisini ilerlet
  if (++pFifo->Head == pFifo->Buffer + pFifo->Size) 
    pFifo->Head = pFifo->Buffer;
  
  pFifo->Full = FALSE;
  
  if (val == '\n')
    --pFifo->nLines;
  
  return val;
}

// '\n' ile karþýlaþana kadar olan blok veriyi çeker ve
// parametre LineBuf'a kopyalar
// Çekilecek line yoksa FALSE döner, varsa TRUE döner
// Line verisinin aktarýldýðý buufer'ýn (LineBuf)
// sonuna '\0' eklenir
int FIFO_GetLine(FIFO *pFifo, unsigned char *LineBuf)
{
  unsigned char val;
  
  if (pFifo->nLines == 0)
    return FALSE;
  
  do {
    val = FIFO_GetData(pFifo);
    *LineBuf++ = val;    
  } while (val != '\n');
  
  *LineBuf = '\0';
  return TRUE;
}





               
               
               