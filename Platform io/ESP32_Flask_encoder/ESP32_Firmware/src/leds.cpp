#include <FastLED.h>
#include "led.h"
#include "pin.h"

// Number of LEDs
const int LEDNum = 12;

CRGB LEDs[LEDNum];

//// LED FUNCTIONS ////

void LEDSet(int idx, const int color[3]) {
    if (idx < 0 || idx >= LEDNum) return;
    LEDs[idx] = CRGB(color[0], color[1], color[2]);
}

void LEDShow() {
    FastLED.show();
}

void LEDInit() {
    FastLED.addLeds<WS2812, PIN_LED, GRB>(LEDs, LEDNum);
}