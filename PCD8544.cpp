/*
 * Author: Borislav Sapundzhiev 
 * Adapted from code by Jim Lindblom, Nathan Seidle and mish-mashed with
 * code from the ColorLCDShield.
 * License: Officially, the MIT License. Review the included License.md file
 * Unofficially, Beerware. Feel free to use, reuse, and modify this
 * code as you see fit. If you find it useful, and we meet someday,
 * you can buy me a beer.
 */
#include "PCD8544.h"

PCD8544::PCD8544(int sce, int rst, int dc, int sdin, int sclk, int bl):
pin_sce(sce), pin_rst(rst), pin_dc(dc), pin_sdin(sdin),pin_sclk(sclk), pin_blight(bl)
{}

void PCD8544::begin() 
{
  pinMode(pin_sce, OUTPUT);
  pinMode(pin_rst, OUTPUT);
  pinMode(pin_dc, OUTPUT);
  pinMode(pin_sdin, OUTPUT);
  pinMode(pin_sclk, OUTPUT);
  //reset display
  digitalWrite(pin_rst, LOW);
  digitalWrite(pin_rst, HIGH);
  //Init
  LcdWrite( LCD_CMD, 0x21 );  // LCD Extended Commands.
  LcdWrite( LCD_CMD, 0xB0 );  // Set LCD Vop (Contrast). //B1
  LcdWrite( LCD_CMD, 0x04 );  // Set Temp coefficent. //0x04
  LcdWrite( LCD_CMD, 0x14 );  // LCD bias mode 1:48. //0x13
  //We must send 0x20 before modifying the display control mode
  LcdWrite( LCD_CMD, 0x20 );  
  LcdWrite( LCD_CMD, 0x0C );  // LCD in normal mode 0x0c. 0x0d for inverse
  
  //backlight
  pinMode(pin_blight, OUTPUT);
  analogWrite(pin_blight, 255);
  
  LcdClear();
}

size_t PCD8544::write (byte c)
{
  LcdWrite(LCD_DATA, 0x00);
  for (int index = 0; index < 5; index++)
  {
    LcdWrite(LCD_DATA, ASCII[c - 0x20][index]);
  }
  LcdWrite(LCD_DATA, 0x00);
  return 1;   // one byte output
}  

void PCD8544::LcdClear()
{
  for (int index = 0; index < LCD_WIDTH * LCD_HEIGHT / 8; index++)
  {
    LcdWrite(LCD_DATA, 0x00);
  }
}

void PCD8544::LcdWrite(byte dc, byte data)
{
  digitalWrite(pin_dc, dc);
  digitalWrite(pin_sce, LOW);
  shiftOut(pin_sdin, pin_sclk, MSBFIRST, data);
  digitalWrite(pin_sce, HIGH);
}

// Set contrast can set the LCD Vop to a value between 0 and 127.
// 40-60 is usually a pretty good range.
void PCD8544::setContrast(byte contrast)
{
  LcdWrite(LCD_CMD, 0x21); //Tell LCD that extended commands follow
  LcdWrite(LCD_CMD, 0x80 | contrast); //Set LCD Vop (Contrast): Try 0xB1(good @ 3.3V) or 0xBF if your display is too dark
  LcdWrite(LCD_CMD, 0x20); //Set display mode
}

void PCD8544::setInverse(bool inverse)
{
   LcdWrite(LCD_CMD, inverse ? 0x0d : 0x0c);
}

void PCD8544::setPower(bool on)
{
    LcdWrite(LCD_CMD, on ? 0x20 : 0x24);
}

// gotoXY routine to position cursor 
// x - range: 0 to 84
// y - range: 0 to 5
void PCD8544::gotoXY(byte x, byte y)
{
  LcdWrite( 0, 0x80 | x);  // Column.
  LcdWrite( 0, 0x40 | y);  // Row.  
}

void PCD8544::drawPX(byte  x, byte  y)
{
  static byte b;
  static byte prev;
  
  byte py = y % 8;
  byte row = (int)(y / 8);
  b = (prev != x) ? 0x0 : b;
  prev = x;
  if ((x < LCD_WIDTH) && (y < LCD_HEIGHT)) {
    b |= 1 << py;
    gotoXY (x, row);
    LcdWrite (LCD_DATA, b);
  }
}

void PCD8544::plotLine(byte x1, byte y1, byte x2, byte y2)
{
    byte deltax = (x2 - x1);     
    byte deltay = (y2 - y1);     
    byte x = x1;                 
    byte y = y1;                 

    byte curpixel, xinc1, xinc2, den, num, numadd, numpixels, yinc1, yinc2;

    if (x2 >= x1)               
    {
      xinc1 = 1;
      xinc2 = 1;
    }
    else                          
    {
      xinc1 = -1;
      xinc2 = -1;
    }

    if (y2 >= y1)               
    {
      yinc1 = 1;
      yinc2 = 1;
    }
    else                          
    {
      yinc1 = -1;
      yinc2 = -1;
    }

    if (deltax >= deltay)       
    {
      xinc1 = 0;                  
      yinc2 = 0;                  
      den = deltax;
      num = deltax / 2;
      numadd = deltay;
      numpixels = deltax;       
    }
    else                          
    {
      xinc2 = 0;                  
      yinc1 = 0;                  
      den = deltay;
      num = deltay / 2;
      numadd = deltax;
      numpixels = deltay;       
    }

    for (curpixel = 0; curpixel <= numpixels; curpixel++)
    {
      drawPX(x, y);         
      num += numadd;              
      if (num >= den)           
      {
        num -= den;         
        x += xinc1;         
        y += yinc1;        
      }
      x += xinc2;               
      y += yinc2;            
    }
}

void PCD8544::plotRect(byte left, byte top, byte right, byte bottom)
{
  plotLine(left, top, right, top); //top
  plotLine(left, bottom, right, bottom); //bottom
  plotLine(right, top, right, bottom); //right
  plotLine(left, top, left, bottom); //left
}

void PCD8544::plotRectFill(byte x0, byte y0, byte x1, byte y1)
{
    byte xDiff;

    if(x0 > x1)
      xDiff = x0 - x1; //Find the difference between the x vars
    else
      xDiff = x1 - x0;

    while(xDiff > 0)
    {
      plotLine(x0, y0, x0, y1);

      if(x0 > x1)
        x0--;
      else
        x0++;

      xDiff--;
    }
}
