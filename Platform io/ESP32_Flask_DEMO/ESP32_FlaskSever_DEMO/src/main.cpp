#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
#include <Wire.h>
#include <iostream>

const char *ssid = "";
const char *password = "";

// LED const
const int PIN_LED = 12;
const int LEDNum = 12;
const int LEDColorConnected[] = {204, 102, 0}; // change the color of the LED light A
const int LEDColorDisconnected[] = {0, 0, 0};

// Test (on/off) colors
const int LEDColorPurple[] = {128, 0, 128};
const int LEDColorTurqoise[] = {83, 195, 189};

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

void setup()
{
  Serial.begin(115200);
  delay(10);
  FastLED.setBrightness(24);

  Serial.println('\n');

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

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void loop()
{
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
