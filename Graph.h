/*
  Graph.h - Library for drawing moving line graphs
  Created by Terry Kolody, 2019.
  Released into the public domain.
*/
#ifndef Graph_h
#define SegGauge_h

#include "Arduino.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735


class Graph
{
  public:
    Graph(Adafruit_ST7735 *tft, int radius, int cenx, int ceny, int seglen, int stepsize, int spacesize, int color, int background);
    void draw(float val);
  private:
    Adafruit_ST7735 *_tft;
    int _radius;
    int _cenx;
    int _ceny;
    int _seglen;
    int _stepsize;
    int _spacesize;
    int _color;
    int _background;
    int _startdeg;
    int _enddeg;
    /*
    _min 
    _max
    _warn
    _warnColor
    _startangle
    _endangle 
    */
};

#endif
