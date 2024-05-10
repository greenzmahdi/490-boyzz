#ifndef OLED_SETUP_H
#define OLED_SETUP_H
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define SCREEN_WIDTH 128 // OLED display width
#define SCREEN_HEIGHT 64 // OLED display height
#define scaleFactor 2

extern int CHAR_WIDTH;

#define maxX 123
#define maxY 59

// Function declarations
void LCDRectFill(int x, int y, int w, int h, int color);
void LCDTextDraw(int x, int y, const char *text, byte size, int colorFont, int colorBG);
void LCDScreenClear();
void LCDInit();

void drawGrid();
void drawPoint(int x, int y);
void updateDisplayWithPoints();
void drawPointOnOLED(int x, int y);
void refreshAndDrawPoints();


#endif