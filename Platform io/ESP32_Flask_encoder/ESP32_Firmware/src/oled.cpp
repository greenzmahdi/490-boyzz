
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "oled.h"

int CHAR_WIDTH = 6;   

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

void LCDLineDraw(int x0, int y0, int x1, int y1, uint16_t color) {
    LCD.drawLine(x0, y0, x1, y1, color);
    // LCD.display(); // Update the display to reflect changes
}

void LCDCircleFill(int x, int y, int radius, uint16_t color) {
    LCD.fillCircle(x, y, radius, color);
    // LCD.display(); // Update the display to reflect changes
}




void drawGrid() {
    // Draw horizontal and vertical axis
    LCDLineDraw(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, WHITE);  // Vertical
    LCDLineDraw(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2, WHITE);  // Horizontal

    // Optionally, add scale marks on axes
}

void drawPoint(int x, int y) {
    // Convert real coordinates to display coordinates
    int displayX = (x * scaleFactor) + (SCREEN_WIDTH / 2);  // Assuming zero is at the center
    int displayY = (SCREEN_HEIGHT / 2) - (y * scaleFactor);

    // Draw point
    LCDCircleFill(displayX, displayY, 2, WHITE);  // Small filled circle as a point
}


void drawPointOnOLED(int x, int y) {
    // Map the x, y coordinates to your display dimensions
    int displayX = map(x, -maxX, maxX, 0, SCREEN_WIDTH);
    int displayY = map(y, -maxY, maxY, SCREEN_HEIGHT, 0);

    LCD.fillCircle(displayX, displayY, 2, WHITE);  // Draw filled circle for point
    LCD.display();  // Refresh the display to show the point
}


