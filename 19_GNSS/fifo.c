#include <stdio.h>
#include <stdint.h>

#include "fifo.h"

// FIFO yap�s�n�n kuyruk (buffer) alan�n� s�f�rlar
void FIFO_Clear(FIFO *pFifo)
{
  pFifo->Head = pFifo->Tail = pFifo->Buffer;
  
  pFifo->nLines = 0;
  pFifo->Full = FALSE;
}

// FIFO yap�s�n� ba�lat�r
void FIFO_Init(FIFO *pFifo, unsigned char *buf, int size)
{
  pFifo->Buffer = buf;
  pFifo->Size = size;

  FIFO_Clear(pFifo);
}

// FIFO yap�s�nda veri yoksa (bo�sa) TRUE 
// veri varsa FALSE
int FIFO_IsEmpty(FIFO *pFifo)
{
  return (pFifo->Head == pFifo->Tail);
}

// FIFO yap�s� t�m�yle doluysa TRUE 
// de�ilse FALSE
int FIFO_IsFull(FIFO *pFifo)
{
  return pFifo->Full;
}

// FIFO yap�s�na veri ekler
// Ekleme ba�ar�l�ysa TRUE d�ner
// Aksi durumda (full) FALSE d�ner
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
  
  *pFifo->Tail = val; // veri y�klendi
  pFifo->Tail = ptr;
  
  if (val == '\n')
    ++pFifo->nLines;
  
  return TRUE;
}

// FIFO yap�s�ndan veri �eker
// Bloke �al���r (�ekilecek veri yoksa)
unsigned char FIFO_GetData(FIFO *pFifo)
{
  unsigned char val;
  
  while (FIFO_IsEmpty(pFifo)) ;
  
  // FIFO da �ekilecek veri var
  val = *pFifo->Head;
  
  // Head g�stericisini ilerlet
  if (++pFifo->Head == pFifo->Buffer + pFifo->Size) 
    pFifo->Head = pFifo->Buffer;
  
  pFifo->Full = FALSE;
  
  if (val == '\n')
    --pFifo->nLines;
  
  return val;
}

// '\n' ile kar��la�ana kadar olan blok veriyi �eker ve
// parametre LineBuf'a kopyalar
// �ekilecek line yoksa FALSE d�ner, varsa TRUE d�ner
// Line verisinin aktar�ld��� buufer'�n (LineBuf)
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





               
               
               