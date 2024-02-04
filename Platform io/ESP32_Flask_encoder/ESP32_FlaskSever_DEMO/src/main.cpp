#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <Wire.h>
#include <iostream>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

const char *ssid = "ADD_YOUR_WIFI";
const char *password = "ADD_YOUR_PASSWORD";

// LED const
const int PIN_LED = 12;
const int LEDNum = 12;
const int LEDColorConnected[] = {204, 102, 0}; // change the color of the LED light A
const int LEDColorDisconnected[] = {0, 0, 0};

// Test (on/off) colors
const int LEDColorPurple[] = {128, 0, 128};
const int LEDColorTurqoise[] = {83, 195, 189};
const int LEDColorGreen[] = {0, 255, 0};

CRGB LEDs[LEDNum];

//// LED FUNCTIONS ////

void LEDSet(const int idx, const int colorR, const int colorG, const int colorB)
{
  if ((idx < 0) || (idx >= LEDNum))
    return;
  if ((colorR < 0) || (colorR >= 256))
    return;
  if ((colorG < 0) || (colorG >= 256))
    return;
  if ((colorB < 0) || (colorB >= 256))
    return;

  LEDs[idx] = CRGB(colorR, colorG, colorB);
}

void LEDSet(const int idx, const int *color)
{
  LEDSet(idx, color[0], color[1], color[2]);
}

void LEDShow()
{
  FastLED.show();
}

void LEDInit()
{
  FastLED.addLeds<WS2812, PIN_LED, GRB>(LEDs, LEDNum);
}

// OLED const
const int PIN_I2C_SCL = 16;
const int PIN_I2C_SDA = 13;
const int RefreshDelay = 5;

const char *MenuItems[] = {"Connection 1 (X)", "Connection 2 (Y)", "Connection 3", "Connection 4", "Connection 5", "Connection 6"};

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

//// I2C FUNCTIONS ////
byte I2CReadRegs(int address, int size)
{
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(address, size);

  return Wire.read();
}

bool I2CReadReg(int address, int size, int idx)
{
  byte regs = I2CReadRegs(address, size);

  return bitRead(regs, idx);
}

//// BUTTON FUNCTIONS ////

bool ButtonRead(int idx)
{
  // 0 - left
  // 1 - center
  // 2 - up
  // 3 - down
  // 4 - right

  if ((idx < 0) || (idx > 4))
    return false;

  return !I2CReadReg(0x20, 1, idx);
}

bool ButtonLeftPressed()
{
  return ButtonRead(0);
}

bool ButtonCenterPressed()
{
  return ButtonRead(1);
}

bool ButtonUpPressed()
{
  return ButtonRead(2);
}

bool ButtonDownPressed()
{
  return ButtonRead(3);
}

bool ButtonRightPressed()
{
  return ButtonRead(4);
}

// Menu setup
bool ButtonStatesPrev[] = {false, false, false, false, false};
int MotorChannelSelected = 0;
int MotorChannelWatched = -1;

//
const int IdxZ1 = 5;
const int IdxZ2 = 4;
const int IdxZ3 = 3;
const int IdxZ4 = 2;
const int IdxZ5 = 1;
const int IdxZ6 = 0;

// PIN setup
const int PIN_A1 = 33;
const int PIN_A2 = 32;
const int PIN_A3 = 35;
const int PIN_A4 = 34;
const int PIN_A5 = 18;
const int PIN_A6 = 27;
const int PIN_B1 = 26;
const int PIN_B2 = 4;
const int PIN_B3 = 17;
const int PIN_B4 = 22;
const int PIN_B5 = 23;
const int PIN_B6 = 14;

// Encoder variables
volatile int encoderPos = 0; // This variable will increase or decrease based on the encoder's rotation
unsigned long lastEncoderRead = 0;

int lastEncoded = 0; // This will store the last state of the encoder

// Encoder functions
void encoderISR()
{
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderRead < 6) // works alright with 6
  {                                      // original: 5 milliseconds debounce time
    return;
  }
  lastEncoderRead = currentTime;

  int newA = digitalRead(PIN_A1);
  int newB = digitalRead(PIN_B1);

  int encoded = (newA << 1) | newB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderPos--;

  lastEncoded = encoded;
}

void setup()
{
  Serial.begin(115200);
  delay(10);

  LEDInit();
  LEDShow();
  for (int i = 0; i < LEDNum; i++)
    LEDSet(i, LEDColorDisconnected);

  // Setting PINs
  pinMode(PIN_I2C_SCL, OUTPUT);
  pinMode(PIN_I2C_SDA, OUTPUT);

  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_A1, INPUT);
  pinMode(PIN_A2, INPUT);
  pinMode(PIN_A3, INPUT);
  pinMode(PIN_A4, INPUT);
  pinMode(PIN_A5, INPUT);
  pinMode(PIN_A6, INPUT);
  pinMode(PIN_B1, INPUT);
  pinMode(PIN_B2, INPUT);
  pinMode(PIN_B3, INPUT);
  pinMode(PIN_B4, INPUT);
  pinMode(PIN_B5, INPUT);
  pinMode(PIN_B6, INPUT);

  // Init OLED
  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin();
  Wire.setClock(400000);
  LCDInit();
  LCDScreenClear();

  // Monitor pin setup
  attachInterrupt(digitalPinToInterrupt(PIN_A1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B1), encoderISR, CHANGE);

  // Dim LEDs
  FastLED.setBrightness(24);

  Serial.println('\n');

  // Connecting to WIFI
  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  // WIFI connection has been established
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());

  // Test screen display when connected
  for (int i = 0; i < LEDNum; i++)
    LEDSet(i, LEDColorGreen);

  LCDTextDraw(10, 6, "ESP32 DRO", 1, 1, 0);
  delay(3000);
}

void loop()
{
  LCDRectFill(70, 54, 20, 8, BLACK); // Adjust the x, y, width, and height as needed

  // Display the encoder position on the LCD
  char buffer[10];
  sprintf(buffer, "%d", encoderPos);
  LCDTextDraw(70, 54, buffer, 1, WHITE, BLACK);

  delayMicroseconds(RefreshDelay);
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://192.168.1.17:5000/status"); // Change Flask server URL to your local wifi ip adress
    int httpCode = http.GET();

    if (httpCode > 0)
    {
      String payload = http.getString();
      LEDInit();
      if (payload == "purple")
      {

        for (int i = 0; i < LEDNum; i++)
          LEDSet(i, LEDColorPurple);
      }
      else if (payload == "turquoise")
      {

        for (int i = 0; i < LEDNum; i++)
          LEDSet(i, LEDColorTurqoise);
      }
      LEDShow();
    }
    else
    {
      Serial.print("Failed to receive HTTP response. Code: ");
      Serial.println(httpCode);
    }

    http.end(); // Close connection
  }
}
