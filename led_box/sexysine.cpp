#include "sexysine.h"

    // All the palettes we want to use
    CRGBPalette16 LAST = CRGBPalette16();
    CRGBPalette16 palettes[] = {
      RainbowColors_p,
      RainbowStripeColors_p,
      OceanColors_p,
      CloudColors_p,
      LavaColors_p,
      ForestColors_p,
      PartyColors_p,
      LAST
    };


sexysine::sexysine(const int width, const int height) {
  _width = width;
  _height = height;
  
  _minValue = 1023;
  _maxValue = 0;

  Serial.begin(38400);

  // To setup Spectrum Analyzer
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(RESET, OUTPUT);
  pinMode(STROBE, OUTPUT);
  digitalWrite(RESET, LOW);
  digitalWrite(STROBE, HIGH);
  
  _leds = (CRGB*)malloc(sizeof(CRGB)*width*height);
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(&(*_leds), width*height);

  // Until power issues are resolved
  FastLED.setBrightness(MAX_BRIGHTNESS);

  // TODO: Get rid of this somehow
  for(int i=0; i<3; i++) {
    _paramList[i].time = 0.0;
    _paramList[i].wait = 0;
  }

  setAllLedsTo(CRGB::Black);
  
  show();
}

CRGB *sexysine::getLeds() {
  return _leds;
}

void sexysine::setAllLedsTo(CRGB color) {
  for(int i=0; i<_width*_height; i++) {
    _leds[i] = color;
  }
}

void sexysine::showRainbow(CRGBPalette16 palette, int paletteSize, int curveSize, int quietSpeed, int sineSize, float brightness, PARAMS *param) {
  int leftVal = _left[0];
  int rightVal = _right[0];

  if(leftVal < _minValue) _minValue = leftVal;
  if(leftVal > _maxValue) _maxValue = leftVal;
  if(rightVal < _minValue) _minValue = rightVal;
  if(rightVal > _maxValue) _maxValue = rightVal;
  
  int leftValue = curveSize*(((float)(leftVal-_minValue)/(float)(_maxValue-_minValue)));
  int rightValue = 5*(((float)(rightVal-_minValue)/(float)(_maxValue-_minValue)));

  for(int x=0; x<_width/2; x++) {
    for(int y=0; y<_height; y++) {
      //sine = leftValue*sin(PI*y/HEIGHT);
      float sine = (float)leftValue*sin16((float)sineSize*(float)y/(float)_height)/(float)sineSize;
      //Serial.println(sin16(65535.0*y/HEIGHT)/32767.0);

      _leds[XY(x,y)] += ColorFromPalette( palette, paletteSize*x/_width + sine + param->wait ).fadeLightBy(brightness);
      _leds[XY(_width-1-x,y)] = _leds[XY(x,y)];
      //leds[XY(x,y)].fadeLightBy(brightness[XY(x,y)]);
    }
  }
  param->time += .3;
  param->wait += rightValue < quietSpeed ? quietSpeed : rightValue;
}

int sexysine::XY(int x, int y) {
  if( x & 0x01 ) {
    return (x * _height) + (_height - 1) - y;
  } else {
    return (x * _height) + y;
  }
}

void sexysine::readMSGEQ7() {
  digitalWrite(RESET, HIGH);
  digitalWrite(RESET, LOW);
  int band;
  for(band=0; band<7; band++) {
    digitalWrite(STROBE, LOW);
    delayMicroseconds(30);
    _left[band] = analogRead(LEFT);
    _right[band] = analogRead(RIGHT);
    
    digitalWrite(STROBE, HIGH);
  }
}

void sexysine::draw(int animation, float brightness) {
  Serial.print("Drawing animation ");
  Serial.println(animation);
  switch(animation) {
    case 0:
      //Serial.print("Before ");
      //Serial.println(paramList[animation].time);
      showRainbow(palettes[1], 64, 40, 1, 65535/2, brightness, &_paramList[animation]);
      //Serial.print("After ");
      //Serial.println(paramList[animation].time);
      break;
    case 1:
      showRainbow(palettes[0], 300, 40, 5, 65535/4, brightness, &_paramList[animation]);
      break;
    case 2:
      showRainbow(palettes[4], 120, 80, 3, 65535/3, brightness, &_paramList[animation]);
      break;
  }
}

void sexysine::show() {
  Serial.println("Showing leds");
  FastLED.show();
}
