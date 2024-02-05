#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
// #include <Wire.h>
#include <iostream>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

// file imports
#include "pin_setup.h"
#include "led_setup.h"
#include "oled_setup.h"

// Wifi credentials
const char *ssid = "[SOSA_HOME]";
const char *password = "armando1!";

// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};

// OLED var const
const int RefreshDelay = 1; // original 5

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"};
const char *MenuDroItems[] = {"Sino", "ToAuto"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};

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
  // PIN SETUP
  setUpPins();

  LEDInit();

  for (int i = 0; i < LEDNum; i++)
    LEDSet(i, LEDColorDisconnected);

  LEDShow();

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

  LCDTextDraw(12, 6, "COMP491 ESP32 DRO", 1, 1, 0);
  // delay(3000);
}

void loop()
{

  // I need to find out why this part of the code is needed for my encoder to displa teh correct data
  LCDRectFill(10, 20, 20, 8, BLACK); // Adjust the x, y, width, and height as needed
  FastLED.setBrightness(14);

  // Display the encoder position on the LCD
  char buffer[10];
  sprintf(buffer, "x: %d", encoderPos);
  LCDTextDraw(10, 20, buffer, 1, WHITE, BLACK);

  // sprintf(buffer, "y: %d", encoderPos);
  // LCDTextDraw(10, 35, buffer, 1, WHITE, BLACK);

  // sprintf(buffer, "z: %d", encoderPos);
  // LCDTextDraw(10, 50, buffer, 1, WHITE, BLACK);

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
          LEDSet(i, LEDColorTurquoise);
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
