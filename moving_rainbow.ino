#include "FastLED.h"
#include "Time.h"
//#include "show.h"

#define STROBE 4
#define RESET 5
#define LEFT 0
#define RIGHT 1
#define MAXINPUT 1023.0

#define WIDTH 24
#define HEIGHT 8
#define NUM_LEDS WIDTH * HEIGHT
#define DATA_PIN 10

#define MAX_BRIGHTNESS 90

CRGB leds[NUM_LEDS];
char brightness[NUM_LEDS];

int left[7];
int right[7];

int minValue = 1023;
int maxValue = 0;

struct PARAMS {
  float time;
  int wait;
};

struct PARAMS paramList[1];

void setup() {
  // For debugging
  Serial.begin(38400);

  // To setup Spectrum Analyzer
  pinMode(LEFT, INPUT);
  pinMode(RIGHT, INPUT);
  pinMode(RESET, OUTPUT);
  pinMode(STROBE, OUTPUT);
  digitalWrite(RESET, LOW);
  digitalWrite(STROBE, HIGH);

  // Initialize LEDs to black
  FastLED.addLeds<WS2811, DATA_PIN, RGB>(leds, NUM_LEDS);
  setAllLedsTo(CRGB::Black);
  
  for(int i=0; i<2; i++) {
    paramList[i].time = 0.0;
    paramList[i].wait = 0;
  }
  
  // Until power issues are resolved
  FastLED.setBrightness(90);

  FastLED.show();
}

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
  
void drawFlag() {
  for(int j=0; j<24; j++) {
    for(int i=0; i<8; i++) {
      if(j<6 || j>17)
        leds[XY(j,i)] = CRGB::Red;
      else
        leds[XY(j,i)] = CRGB::White;
    }
  }
  leds[XY(11,7)] = CRGB::Red;
  leds[XY(12,7)] = CRGB::Red;
  for(int i=7; i<17; i++) {
    leds[XY(i,6)] = CRGB::Red;
    leds[XY(i,4)] = CRGB::Red;
  }
  for(int i=9; i<15; i++) {
    leds[XY(i,5)] = CRGB::Red;
  }
  for(int j=1; j<=3; j++) {
    for(int i=10; i<14; i++)
      leds[XY(i,j)] = CRGB::Red;
  }
  leds[XY(10,0)] = CRGB::Red;
  leds[XY(13,0)] = CRGB::Red;
}

// Needs http://www.pjrc.com/teensy/td_libs_Time.html
int lastTime = now();
int currentDisplay = 0;
int previousDisplay = 0;
float currentBrightness = 1.0;
float previousBrightness = 0;
int transitionTime = 0;
boolean inTransition = false;

#define DELAY_BETWEEN_ANIMATIONS 10
#define TRANSITION_DURATION 5.0

void loop() {
  // Read the audio data
  readMSGEQ7();
  
  // Clear the old values
  setAllLedsTo(CRGB::Black);

  int currentTime = now();
  if(inTransition && currentTime > transitionTime + TRANSITION_DURATION) {
    Serial.println("Ending transition");
    inTransition = false;
    currentBrightness = 256;
  }

  // Setup LEDs
  draw(currentDisplay, currentBrightness);
  //draw(0);
  //drawFlag();
  
  if(inTransition) {
    draw(previousDisplay, previousBrightness);
    // If we are in transition we just want to fade the previous display by a certain percentage
    // and do the opposite with the new one
    previousBrightness = 255*(float)(currentTime - lastTime)/TRANSITION_DURATION;
    currentBrightness = 255-previousBrightness;
    Serial.print(previousBrightness);
    Serial.print(", ");
    Serial.println(currentBrightness);
  }

  // Check if time is up for this display
  if(currentTime > lastTime + DELAY_BETWEEN_ANIMATIONS) {
    Serial.println("Starting transition");
    lastTime = currentTime;
    previousDisplay = currentDisplay;
    if(currentDisplay == 1) {
      currentDisplay = 0;
    } else {
      currentDisplay++;
    }
    inTransition = true;
    transitionTime = now();
  }
  
  // Draw
  FastLED.countFPS(60);
  FastLED.show();
}

void draw(int animation, float brightness) {
  switch(animation) {
    case 0:
      showRainbow(palettes[1], 64, 40, 1, 65535/2, brightness, paramList[animation]);
      break;
    case 1:
      showRainbow(palettes[0], 300, 40, 5, 65535/4, brightness, paramList[animation]);
      break;
  }
}

void showRainbow(CRGBPalette16 palette, int paletteSize, int curveSize, int quietSpeed, int sineSize, float brightness, struct PARAMS param) {
  int leftVal = left[0];
  int rightVal = right[0];

  if(leftVal < minValue) minValue = leftVal;
  if(leftVal > maxValue) maxValue = leftVal;
  if(rightVal < minValue) minValue = rightVal;
  if(rightVal > maxValue) maxValue = rightVal;
  
  int leftValue = curveSize*(((float)(leftVal-minValue)/(float)(maxValue-minValue)));
  int rightValue = 5*(((float)(rightVal-minValue)/(float)(maxValue-minValue)));

  for(int x=0; x<WIDTH/2; x++) {
    for(int y=0; y<HEIGHT; y++) {
      //sine = leftValue*sin(PI*y/HEIGHT);
      float sine = (float)leftValue*sin16((float)sineSize*(float)y/(float)HEIGHT)/(float)sineSize;
      //Serial.println(sin16(65535.0*y/HEIGHT)/32767.0);

      leds[XY(x,y)] += ColorFromPalette( palette, paletteSize*x/WIDTH + sine + param.wait ).fadeLightBy(brightness);
      //leds[XY(WIDTH-x,y)] = leds[XY(x,y)];
      //leds[XY(x,y)].fadeLightBy(brightness[XY(x,y)]);
    }
  }
  param.time += .3;
  param.wait += rightValue < quietSpeed ? quietSpeed : rightValue;
}

void setAllLedsTo(CRGB color) {
  for(int i=0; i<NUM_LEDS; i++) {
    leds[i] = color;
  }
}

int XY(int x, int y) {
  if( x & 0x01 ) {
    return (x * HEIGHT) + (HEIGHT - 1) - y;
  } else {
    return (x * HEIGHT) + y;
  }
}

void readMSGEQ7() {
  digitalWrite(RESET, HIGH);
  digitalWrite(RESET, LOW);
  int band;
  for(band=0; band<7; band++) {
    digitalWrite(STROBE, LOW);
    delayMicroseconds(30);
    left[band] = analogRead(LEFT);
    right[band] = analogRead(RIGHT);
    
    digitalWrite(STROBE, HIGH);
  }
}

