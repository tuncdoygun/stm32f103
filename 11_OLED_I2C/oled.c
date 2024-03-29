#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "system.h"
#include "io.h"
#include "hi2c.h"

#include "oled.h"

#include "font_5x8.h"
#include "font_7x16.h"

#define I2C_PORT        I2C_1
#define I2C_ADR_OLED    0x78

#define PI      3.141593654

#define T_COLS  (NSEG / ((_szWidth) + 1))
#define T_ROWS  (NPGS / (_szHeight))

static int      _szWidth;       // aktif font pixel geni�li�i (bit)
static int      _szHeight;      // aktif font y�ksekli�i (page - byte)
static const unsigned char      *_pFont; // aktif font tablosu

static int      _row, _col;     // text sat�r, s�tun
static int      _font;

static int      _page, _segment;

uint8_t  _DspRam[NPGS * NSEG];

// SSD1306 OLED display'e komut g�nderir
void OLED_Command(uint8_t cmd)
{
  uint8_t buf[2];
  
  buf[0] = 0x00;        // Command mode
  buf[1] = cmd;
  
  HI2C_Write(I2C_PORT, I2C_ADR_OLED, buf, 2);  
}

// OLED display'e pixel data g�nderir
// Her data byte 8 pixele kar��l�k gelir
void OLED_Data(uint8_t data)
{
  uint8_t buf[2];
  
  buf[0] = 0x40;        // Data mode
  buf[1] = data;
  
  HI2C_Write(I2C_PORT, I2C_ADR_OLED, buf, 2);  
  
  _DspRam[_page * NSEG + _segment] = data;
  
  if (++_segment >= NSEG)
    _segment = 0;
  
}

void OLED_SetPage(uint8_t page)
{
  page &= 0x07;
  
  _page = page;
  OLED_Command(0xB0 | page);
}

void OLED_SetSegment(uint8_t segment)
{
  segment &= 0x7F;
  
  _segment = segment;
  
  OLED_Command(segment & 0x0F);
  OLED_Command(0x10 | (segment >> 4));
}

void OLED_FillPage(uint8_t page, uint8_t ch)
{
  int i;
  
  OLED_SetPage(page);
  OLED_SetSegment(0);
  
  for (i = 0; i < NSEG; ++i)
    OLED_Data(ch);
}

void OLED_FillDisplay(uint8_t ch)
{
  int page;
  
  for (page = 0; page < NPGS; ++page)
    OLED_FillPage(page, ch);
}

void OLED_ClearDisplay(void)
{
  OLED_FillDisplay(0);
}

void OLED_Rotate(int bRotate)
{
  unsigned char remap, scan;
  
  if (bRotate) {
    remap = 0xA0;
    scan = 0xC0;
  }
  else {
    remap = 0xA1;
    scan = 0xC8;
  }

  OLED_Command(remap); // Set Segment Re-Map
  OLED_Command(scan); // Set Com Output Scan Direction
}

void OLED_Start(int bRotate)
{
  HI2C_Init(I2C_PORT, 100000);
  
#ifdef IO_OLED_GND
  IO_Write(IOP_OLED_GND, 0);
  IO_Init(IOP_OLED_GND, IO_MODE_OUTPUT);
#endif  

#ifdef IO_OLED_VDD
  IO_Write(IOP_OLED_VDD, 1);
  IO_Init(IOP_OLED_VDD, IO_MODE_OUTPUT);
#endif  
  
  DelayMs(80);

  //////////////////////////////////////
  
  OLED_Command(0xAE); // Set display OFF		

  OLED_Command(0xD4); // Set Display Clock Divide Ratio / OSC Frequency
  OLED_Command(0x80); // Display Clock Divide Ratio / OSC Frequency 

  OLED_Command(0xA8); // Set Multiplex Ratio
  OLED_Command(0x3F); // Multiplex Ratio for 128x64 (64-1)

  OLED_Command(0xD3); // Set Display Offset
  OLED_Command(0x00); // Display Offset

  OLED_Command(0x40); // Set Display Start Line

  OLED_Command(0x8D); // Set Charge Pump
  OLED_Command(0x14); // Charge Pump (0x10 External, 0x14 Internal DC/DC)

  OLED_Rotate(bRotate);

  OLED_Command(0xDA); // Set COM Hardware Configuration
  OLED_Command(0x12); // COM Hardware Configuration

  OLED_Command(0x81); // Set Contrast
  OLED_Command(0x80); // Contrast

  OLED_Command(0xD9); // Set Pre-Charge Period
  OLED_Command(0xF1); // Set Pre-Charge Period (0x22 External, 0xF1 Internal)

  OLED_Command(0xDB); // Set VCOMH Deselect Level
  OLED_Command(0x40); // VCOMH Deselect Level

  OLED_Command(0xA4); // Enable display outputs according to the GDDRAM contents
  OLED_Command(0xA6); // Set display not inverted
    
  OLED_ClearDisplay();

  OLED_Command(0xAF); // Set display On
  
  //////////////////////////////////////
  // Set default font
  OLED_SetFont(FNT_SMALL);
}

void OLED_UpdateDisplay(void)
{
  int i, page, segment;
  
  for (i = page = 0; page < NPGS; ++page) {
    OLED_SetPage(page);
    OLED_SetSegment(0);
    
    for (segment = 0; segment < NSEG; ++segment)
      OLED_Data(_DspRam[i++]);
  }
}

void OLED_SetFont(int font)
{
  switch (font) {
  case FNT_SMALL:
    _szWidth = 5;
    _szHeight = 1;
    _pFont = g_ChrTab;
    _font = FNT_SMALL;
    break;
    
  case FNT_LARGE:
    _szWidth = 7;
    _szHeight = 2;
    _pFont = g_ChrTab2;
    _font = FNT_LARGE;
    break;
  }
}

void OLED_Scroll(int nLines)
{
  int i, j;
  
  // i: Kopya hedefi
  
  // j: Kopya kayna��
  j = nLines * NSEG;
  
  for (i = 0; i < (NPGS - nLines) * NSEG; ++i)
    _DspRam[i] = _DspRam[j++];
  
  for ( ; i < NPGS * NSEG; ++i)
    _DspRam[i] = 0;
  
  OLED_UpdateDisplay();
}

int OLED_GetFont(void)
{
  return _font;
}

void OLED_SetCursor(int row, int col)
{
  _row = row;
  _col = col;
}

void OLED_GetCursor(int *pRow, int *pCol)
{
  *pRow = _row;
  *pCol = _col;
}

void OLED_DrawBitmap(const uint8_t *pBitmap)
{
  int i;
  
  for (i = 0; i < NPGS * NSEG; ++i)
    _DspRam[i] = pBitmap[i];
  
  OLED_UpdateDisplay();
}

void OLED_Return(void)
{
  _col = 0;
}

void OLED_NewLine(void)
{
  if (++_row >= T_ROWS) {
    _row = T_ROWS - 1;
    
    OLED_Scroll(_szHeight);
  }
}

void OLED_PutChar(char c)
{
  int i, j, k;
    
  if (_col >= T_COLS) {
    OLED_Return();
    OLED_NewLine();
  }
  
  for (k = 0; k < _szHeight; ++k) {
    OLED_SetPage(_row * _szHeight + k);
    OLED_SetSegment(_col * (_szWidth + 1));
    
    i = _szWidth * _szHeight * c + k;
    
    for (j = 0; j < _szWidth; ++j) {
      OLED_Data(_pFont[i]);
      i += _szHeight;
    }
                
    OLED_Data(0);
  }
                
  ++_col;
}

////////////////////////////////////////////////////////////////////////////////

void OLED_PixelData(int x, int y, int c)
{
  uint8_t page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  //page = y / 8;
  
  bitIdx = y & 7;
  //bitIdx = y % 8;
  
  val = _DspRam[page * NSEG + x];
  
  switch (c) {
  case OLED_SETPIXEL:
    val |= (1 << bitIdx);
    break;
    
  case OLED_CLRPIXEL:
    val &= ~(1 << bitIdx);
    break;
    
  case OLED_INVPIXEL:
    val ^= (1 << bitIdx);
    break;
  }
  
  _DspRam[page * NSEG + x] = val;
}

void OLED_SetPixel(int x, int y, int c)
{
  uint8_t page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  //page = y / 8;
  
  bitIdx = y & 7;
  //bitIdx = y % 8;
  
  val = _DspRam[page * NSEG + x];
  
  switch (c) {
  case OLED_SETPIXEL:
    val |= (1 << bitIdx);
    break;
    
  case OLED_CLRPIXEL:
    val &= ~(1 << bitIdx);
    break;
    
  case OLED_INVPIXEL:
    val ^= (1 << bitIdx);
    break;
  }
  
  OLED_SetPage(page);
  OLED_SetSegment(x);
  OLED_Data(val);
}

int OLED_GetPixel(int x, int y)
{
  uint8_t page, bitIdx, val;
  
  x &= 0x7F;
  y &= 0x3F;
  
  page = y >> 3;
  bitIdx = y & 7;

  val = _DspRam[(unsigned)page * NSEG + x];
  
  return (val & (1 << bitIdx)) != 0;
}

#define abs(a)      (((a) > 0) ? (a) : -(a))

void OLED_Line(int x0, int y0, int x1, int y1, int c)
{
     int steep, t ;
     int deltax, deltay, error;
     int x, y;
     int ystep;

     steep = abs(y1 - y0) > abs(x1 - x0);

     if (steep)
     { // swap x and y
         t = x0; x0 = y0; y0 = t;
         t = x1; x1 = y1; y1 = t;
     }

     if (x0 > x1)
     {  // swap ends
         t = x0; x0 = x1; x1 = t;
         t = y0; y0 = y1; y1 = t;
     }

     deltax = x1 - x0;
     deltay = abs(y1 - y0);
     error = 0;
     y = y0;

     if (y0 < y1) 
         ystep = 1;
     else
         ystep = -1;

     for (x = x0; x < x1; x++)
     {
         if (steep)
            OLED_SetPixel(y, x, c);
         else
            OLED_SetPixel(x, y, c);

         error += deltay;
         if ((error << 1) >= deltax)
         {
             y += ystep;
             error -= deltax;
         } // if
     } // for
} // line

void OLED_Circle(int x, int y, int r, int c)
{
    float step, t;
    int dx, dy;

    step = PI / 2 / 64;

    for (t = 0; t <= PI / 2; t += step) {
        dx = (int)(r * cos(t) + 0.5);
        dy = (int)(r * sin(t) + 0.5);

        if (x + dx < 128) {
            if (y + dy < 64)
                OLED_SetPixel(x + dx, y + dy, c);
            if (y - dy >= 0)
                OLED_SetPixel(x + dx, y - dy, c);
        }
        if (x - dx >= 0) {
            if (y + dy < 64)
                OLED_SetPixel(x - dx, y + dy, c);
            if (y - dy >= 0)
                OLED_SetPixel(x - dx, y - dy, c);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////

void OLED_putch(char c)
{
  switch (c) {
  case '\n':
    OLED_NewLine();
    
  case '\r':
    OLED_Return();
    break;
    
  case '\f':
    OLED_ClearDisplay();
    break;
    
  default:
    OLED_PutChar(c);
    break;
  }
}

