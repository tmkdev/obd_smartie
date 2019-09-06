/*
  Graph.h - Library for drawing moving line graphs
  Created by Terry Kolody, 2019.
  Released into the public domain.
*/
#ifndef Graph_h
#define Graphe_h

#include "Arduino.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735


class Graph
{
  public:
    Graph(Adafruit_ST7735 *tft, String title, float minval, float maxval, int tracecolor);
    void draw(float val);
  private:
    Adafruit_ST7735 *_tft;
    String _title;
    float _minval;
    float _maxval;
    int _tracecolor;
};

#endif
