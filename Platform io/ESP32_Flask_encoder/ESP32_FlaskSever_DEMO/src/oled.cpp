#include <Adafruit_SSD1306.h>
# include "oled_setup.h"
// OLED setup
Adafruit_SSD1306 LCD(128, 64, &Wire);

// OLED FUNCTIONS
void LCDRectFill(int x, int y, int w, int h, int color)
{
  LCD.fillRect(x, y, w, h, color);
}

void LCDTextDraw(int x, int y, const char *text, byte size, int colorFont, int colorBG)
{
  LCD.setCursor(x, y);
  LCD.setTextSize(size);
  LCD.setTextColor(colorFont, colorBG);
  LCD.print(text);
  LCD.display();
}

void LCDScreenClear()
{
  LCD.clearDisplay();
  LCD.display();
  LCD.setTextColor(WHITE, BLACK);
}

void LCDInit()
{
  LCD.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}