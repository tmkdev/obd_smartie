/*
 SegGuage.cpp
*/

#include "Arduino.h"
#include "SegGauge.h"
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

SegGauge::SegGauge(Adafruit_ST7735 *tft, int radius, int cenx, int ceny, int seglen, int stepsize, int spacesize, int color, int background)
{
  _tft = tft;
  _radius = radius;
  _cenx = cenx;
  _ceny = ceny;
  _seglen = seglen;
  _stepsize = stepsize;
  _spacesize = spacesize;
  _color = color;
  _background = background;
  _startdeg=90;
  _enddeg=200;
}

void SegGauge::draw(float val)
{
  for (int degstep=_startdeg; degstep <= _enddeg; degstep += _stepsize) {
    int segcolor = _color;
    if ( (float(degstep - _startdeg) / (_enddeg - _startdeg + _stepsize ) ) >= val ) {
      segcolor = _background;
      
    }
    
    int x1 = _cenx - _rpx( (degstep - _stepsize/2) + _spacesize, _radius);
    int y1 = _ceny + _rpy( (degstep - _stepsize/2) + _spacesize, _radius);
    int x2 = _cenx - _rpx( (degstep + _stepsize/2) - _spacesize, _radius);
    int y2 = _ceny + _rpy( (degstep + _stepsize/2) - _spacesize, _radius);
    int x3 = _cenx - _rpx( (degstep + _stepsize/2) - _spacesize, _radius - _seglen);
    int y3 = _ceny + _rpy( (degstep + _stepsize/2) - _spacesize, _radius - _seglen);
    int x4 = _cenx - _rpx( (degstep - _stepsize/2) + _spacesize, _radius - _seglen);
    int y4 = _ceny + _rpy( (degstep - _stepsize/2) + _spacesize, _radius - _seglen);

    _tft->fillTriangle(x1,y1,x2,y2,x3,y3,segcolor);
    _tft->fillTriangle(x1,y1,x3,y3,x4,y4,segcolor);
  }

}

int SegGauge::_rpx(int deg, int radius)
{
  return int(round( sin(radians(deg)) * radius ));
}

int SegGauge::_rpy(int deg, int radius)
{
  return int(round( cos(radians(deg)) * radius ));
}
