#include <FastLED.h>
#include "led.h"
#include "pin.h"

// Number of LEDs
const int LEDNum = 12;
// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};

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

void turnOffLEDs(){
    LEDInit();

    for (int i = 0; i < LEDNum; i++)
        LEDSet(i, LEDColorDisconnected);

    LEDShow();

    FastLED.setBrightness(50);
}
