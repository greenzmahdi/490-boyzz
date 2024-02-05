#ifndef OLED_SETUP_H
#define OLED_SETUP_H
#include <Adafruit_GFX.h>

// Function declarations
void LCDRectFill(int x, int y, int w, int h, int color);
void LCDTextDraw(int x, int y, const char *text, byte size, int colorFont, int colorBG);
void LCDScreenClear();
void LCDInit();

#endif