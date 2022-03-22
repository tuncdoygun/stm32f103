#ifndef _ERTC_H
#define _ERTC_H

typedef struct {
  uint8_t sec;
  uint8_t min;
  uint8_t hour;
  
  uint8_t day;    // Haftanýn günü 1..7
  uint8_t date;   // Ayýn günü 1..3
  uint8_t mon;
  uint8_t year;   // 0..99
  
  uint8_t a1_sec;
  uint8_t a1_min;
  uint8_t a1_hour;
  uint8_t a1_day_date;  
  
  uint8_t a2_min;
  uint8_t a2_hour;
  uint8_t a2_day_date;
  
  uint8_t ctl;
  uint8_t stat;
  uint8_t aging;
  
  int16_t       temp;   // sýcaklýk
} ERTC_DATE;

void ERTC_GetDate(ERTC_DATE *pdt, int regAdr, int len);
void ERTC_SetDate(ERTC_DATE *pdt, int regAdr, int len);
void ERTC_Init(void);

#endif
