#include "FastLED.h"
#include "Time.h"
//#include "show.h"
#include "sexysine.h"

#define MAXINPUT 1023.0

#define DATA_PIN 13

#define BUTTON1 9
#define BUTTON2 10
#define DIAL1 4
#define DIAL2 5

#define WIDTH 24
#define HEIGHT 8
#define NUM_LEDS WIDTH * HEIGHT

#define SWITCH_ENABLED false

char brightness[NUM_LEDS];
  

CRGB *leds;

sexysine *_sexysine;

void setup() {
  // For debugging

  pinMode(BUTTON1, INPUT);
  pinMode(BUTTON2, INPUT);
  //pinMode(DIAL1, INPUT);
  
  _sexysine = new sexysine(WIDTH, HEIGHT);
  leds = _sexysine->getLeds();
  
  //FastLED.show();
}


// Needs http://www.pjrc.com/teensy/td_libs_Time.html
int lastTime = now();
int currentDisplay = 1;
int previousDisplay = 0;
float currentBrightness = 1.0;
float previousBrightness = 0;
int transitionTime = 0;
boolean inTransition = false;

#define DELAY_BETWEEN_ANIMATIONS 10
#define TRANSITION_DURATION 5.0

int lastDial1 = 0;
boolean changeAnimation = false;
float speedDial = 1.0;
boolean button1Down = false;

void loop() {
  // Read the audio data
  _sexysine->readMSGEQ7();

  int currentTime = now();
  
  // Check for button press
  if(digitalRead(BUTTON1) == HIGH) {
    if(button1Down == false) {
      Serial.println("Button1 was pressed");
      changeAnimation = true;
      button1Down = true;
    }
  } else {
    button1Down = false;
  }
  
  if(digitalRead(BUTTON2) == HIGH) {
    Serial.println("Button2 was pressed");
  }
  int dial1 = analogRead(DIAL1);
  if(dial1 > 900) dial1 = 900;
  speedDial = 1.0 + 3.0*((float)dial1/900.0);
  //Serial.println(speedDial);
  
  // Clear the old values
  _sexysine->setAllLedsTo(CRGB::Black);

  if(inTransition && currentTime > transitionTime + TRANSITION_DURATION) {
    inTransition = false;
    currentBrightness = 256;
  }

  // Setup LEDs
  _sexysine->draw(currentDisplay, currentBrightness);
  //draw(0);
  //drawFlag();
  
  if(inTransition) {
    _sexysine->draw(previousDisplay, previousBrightness);
    // If we are in transition we just want to fade the previous display by a certain percentage
    // and do the opposite with the new one
    previousBrightness = 255*(float)(currentTime - lastTime)/TRANSITION_DURATION;
    currentBrightness = 255-previousBrightness;
  }

  // Check if time is up for this display
  if(changeAnimation || (SWITCH_ENABLED && currentTime > lastTime + DELAY_BETWEEN_ANIMATIONS)) {
  //if(changeAnimation) {
    changeAnimation = false;
    lastTime = currentTime;
    previousDisplay = currentDisplay;
    if(currentDisplay == 2) {
      currentDisplay = 0;
    } else {
      currentDisplay++;
    }
    if(SWITCH_ENABLED) {
      inTransition = true;
    }
    transitionTime = now();
  }
  
  // Draw
  //FastLED.countFPS(60);
  _sexysine->show(); //FastLED.show();
}


