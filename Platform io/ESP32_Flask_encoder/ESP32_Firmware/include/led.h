#ifndef LED_SETUP_H
#define LED_SETUP_H

#include <FastLED.h>

// Number of LEDs
extern const int LEDNum;

// Function declarations
void LEDSet(int idx, const int color[3]);
void LEDShow();
void LEDInit();

void turnOffLEDs();

#endif
