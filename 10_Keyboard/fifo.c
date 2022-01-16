#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "fifo.h"

void FIFO_Init(FIFO *pFifo, FIFO_VAR *Buffer, unsigned Length)
{
  pFifo->Buffer = Buffer;
  pFifo->Length = Length;
  pFifo->pHead = Buffer;
  pFifo->PTail = Buffer;
}

// Birinci parametrede belirtilen kuyruðun sonuna
// ikinci parametredeki deðeri ekler
// Geri dönüþ true ise baþarýlý
// false ise baþarýsýz (buffer full)
_Bool FIFO_SetData(FIFO *pFifo, FIFO_VAR data)
{
  // ptr kopya gösterici (pTail için)
  FIFO_VAR *ptr = pFifo->PTail;
  
  // ptr ring buffer kuralýna uygun artýrýlýyor
  if (++ptr == pFifo->Buffer + pFifo->Length)
    ptr = pFifo->Buffer;
  
  // Eðer pTail artýrýldýktan sonra pHead'e eþit oluyorsa
  // buffer full demektir, ekleme yapamayýz
  if (ptr == pFifo->pHead) {
    // Beep();
    return false;
  }
  
  *pFifo->PTail = data;
  pFifo->PTail = ptr;
  
  return true;
}

// FIFO kuyruk boþsa true döner
int FIFO_IsEmpty(FIFO *pFifo)
{
  return (pFifo->pHead == pFifo->PTail);
}

// FIFO buffer'dan veri çeker (kuyruðun baþýndan)
// Eðer buffer boþsa bloke bekler
FIFO_VAR FIFO_GetData(FIFO *pFifo)
{
  FIFO_VAR data;
  
  // Kuyruk boþ olduðu müddetçe bekle
  while (FIFO_IsEmpty(pFifo)) ;
  
  data = *pFifo->pHead;
  
  // dairesel buffer kuralýna uygun olarak
  // pHead göstericisini artýracaðýz
  if (++pFifo->pHead == pFifo->Buffer + pFifo->Length)
    pFifo->pHead = pFifo->Buffer;
  
  return data;
}