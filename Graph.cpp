/*
 Graph.cpp
*/

#include "Arduino.h"
#include "Graph.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

int lastx = 0;

Graph::Graph(Adafruit_ST7735 *tft, String title, float minval, float maxval, int tracecolor)
{
    _tft = tft;
    _title = title;
    _minval = minval;
    _maxval = maxval;
    _tracecolor = tracecolor;
}

void Graph::draw(float val)
{
  
    _tft->setTextColor(ST77XX_WHITE, ST77XX_BLACK);
    
    _tft->setTextSize(1);
    _tft->setCursor(80 - (_title.length() * 6) / 2, 0); 
    _tft->print(_title);

    _tft->setCursor(0, 0); 
    _tft->print(String(_maxval, 2));

    _tft->setCursor(0, 121); 
    _tft->print(String(_minval, 2));

    _tft->setCursor(130, 121); 
    _tft->print(String(val, 2));

    _tft->drawLine(0, 12, 160, 12, ST77XX_WHITE);
    _tft->drawLine(0, 116, 160, 116, ST77XX_WHITE);

    _tft->drawLine(lastx, 13, lastx, 115, ST77XX_BLACK);
    _tft->drawLine(lastx+2, 13, lastx+2, 115, ST77XX_WHITE);

    int ypoint = (102 * (val / (_maxval - _minval))) ;

    lastx += 1;
    lastx = lastx % 160;
    _tft->fillCircle(lastx, 115-ypoint, 2, _tracecolor);

}
