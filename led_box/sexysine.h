#ifndef __sexysine_h__
#define __sexysine_h__

#include "Arduino.h"
#include "FastLED.h"

#define STROBE 4
#define RESET 5
#define LEFT 0
#define RIGHT 1
#define DATA_PIN 13
#define MAX_BRIGHTNESS 90

typedef struct {
  float time;
  int wait;
} PARAMS;

class sexysine {
  public:
    sexysine(int width, int height);
    CRGB* getLeds();
    void setAllLedsTo(CRGB color);
    void readMSGEQ7();
    void draw(int animation, float brightness);
    void show();

  private:
    int XY(int x, int y);
    void showRainbow(CRGBPalette16 palette, int paletteSize, int curveSize, int quietSpeed, int sineSize, float brightness, PARAMS *param);
    int _width;
    int _height;
    uint8_t _pin;
    CRGB* _leds;
    int _left[7];
    int _right[7];
    int _minValue;
    int _maxValue;
    PARAMS _paramList[2];

};

#endif

