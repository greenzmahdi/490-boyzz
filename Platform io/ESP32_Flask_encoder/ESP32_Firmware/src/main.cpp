#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <HTTPClient.h>
#include <SPIFFS.h>

#include <FastLED.h>
// #include <Wire.h>
#include <iostream>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>
#include <cmath>
#include <vector> // Include the C++ standard vector library

// file imports
#include "I2C.h"
#include "pin.h"
#include "led.h"
#include "oled.h"
#include "encoder.h"
#include "buttons.h"
#include "format.h"
#include "menu.h"
#include "coordinatePlanes.h"
#include "networkConfig.h"


#define SCREEN_WIDTH 128 // OLED display width
// #define CHAR_WIDTH 6     // Width of each character in pixels

// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};

// Forward declarations
void TaskNetwork(void *pvParameters);
void TaskUpdateDisplay(void *pvParameters);


const int ledPin = 12;  // The GPIO pin connected to your LED strip
const int numLeds = 12; // Number of LEDs in your strip

CRGB leds[numLeds];


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

    FastLED.setBrightness(50);

    // Init OLED
    Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.begin();
    Wire.setClock(400000);
    LCDInit();
    LCDScreenClear();

    // Initialize LED strip
    FastLED.addLeds<WS2812B, ledPin, GRB>(leds, numLeds);
    FastLED.setBrightness(50);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }

    

    setupAccessPoint(); // Set up WiFi access point
    setupWebServerRoutes(); // Set up web server routes

    // Monitor pin setup //
    attachInterrupt(digitalPinToInterrupt(encoder1.pinA), handleEncoder1Interrupt, CHANGE); // I need to remove the lambda and include the w/ name of interrupt to ensure we are using the correct one

    attachInterrupt(digitalPinToInterrupt(encoder1.pinB), handleEncoder1Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder2.pinA), handleEncoder2Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder2.pinB), handleEncoder2Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder3.pinA), handleEncoder3Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder3.pinB), handleEncoder3Interrupt, CHANGE);

    // Dim LEDs
    FastLED.setBrightness(24);

    xTaskCreate(
        TaskUpdateDisplay, // Task function
        "DisplayTask",     // Name of the task
        10000,             // Stack size of task
        NULL,              // Parameter of the task
        1,                 // Priority of the task
        NULL);             // Task handle

    // // delay(3000);
}

void loop() {} // might not need this


void TaskUpdateDisplay(void *pvParameters)
{
    for (;;)
    {
        handleMenuNavigation();
        updateDisplayContent();
        // vTaskDelay(pdMS_TO_TICKS(100));
    }
}
