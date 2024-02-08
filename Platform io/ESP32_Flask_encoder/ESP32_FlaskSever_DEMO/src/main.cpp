/*
Notes:
Task Stack Sizes:

Ensure that the stack size (10000) is sufficient for the tasks' needs. Too small and you'll run into stack overflows; too large and you're wasting precious memory. This often requires some trial and error to get right.
Wi-Fi Connection Handling:

After the initial connection to Wi-Fi, there's no handling for potential Wi-Fi disconnections within the TaskNetwork. It would be beneficial to implement a reconnection strategy.
Global Variable Access:

The encoderPos variable is accessed from both the ISR and TaskUpdateDisplay. It's declared volatile, which is good, but in a multitasking environment, you might need to protect its access with a mutex to avoid race conditions.
Resource Management:

If you're using I2C or any shared resource in both tasks, ensure you manage concurrent access properly, potentially with semaphores.
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
// #include <Wire.h>
#include <iostream>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>

// file imports
#include "pin_setup.h"
#include "led_setup.h"
#include "oled_setup.h"

// Wifi credentials
const char *ssid = "randy.herrera.928@my.csun.edu";
const char *password = "meandcsun0210!";

// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};

// OLED var const
const int RefreshDelay = 1; // original 5

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"}; // might not need this, depends on design
const char *MenuDroItems[] = {"Sino", "ToAuto"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};

// Forward declarations
void TaskNetwork(void *pvParameters);
void TaskUpdateDisplay(void *pvParameters);

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

void monitorPins(int A, int B){
  int encoded = (A << 1) | B;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderPos--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderPos++;

    lastEncoded = encoded;
}

// Encoder functions
void encoderISR()
{
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderRead < 3.4) // works alright with 6
  {                                        // original: 5 milliseconds debounce time
    return;
  }
  lastEncoderRead = currentTime;

  int newA1 = digitalRead(PIN_A1);
  int newB1 = digitalRead(PIN_B1);

  monitorPins(newA1, newB1);

  // int encoded = (newA << 1) | newB;
  // int sum = (lastEncoded << 2) | encoded;

  // if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
  //   encoderPos--;
  // if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
  //   encoderPos++;

  
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

  // Handle display updates here
  LCDTextDraw(12, 6, "COMP491 ESP32 DRO", 1, 1, 0);

  // Serial.println('\n');
  xTaskCreate(
      TaskNetwork,   // Task function
      "NetworkTask", // Name of the task
      10000,         // Stack size of task
      NULL,          // Parameter of the task
      1,             // Priority of the task
      NULL);         // Task handle

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

void TaskNetwork(void *pvParameters)
{
  // Initial connection attempt
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected.");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Dim LEDs
  FastLED.setBrightness(14);

  // Task loop
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
    
    /* creating test JSON data*/
      int outgoingvalue = 123;
      StaticJsonDocument<200> doc;
      doc["outgoingvalue"] = outgoingvalue;
      String jsonstring;
      serializeJson(doc, jsonstring);
    
    // If connected, perform HTTP operations

      
      HTTPClient http1;
      HTTPClient http2;

      http1.begin("http://127.0.0.1:5000/getposition");
      http1.addHeader("Content-Type", "application/json");
      int httpCode = http1.POST(jsonstring);
      String payload = http1.getString();
      http1.end();

      
      http2.begin("http://192.168.1.17:5000/status"); // Your server URL
      httpCode = http2.GET();

      if (httpCode > 0)
      {
        String payload = http2.getString();
        LEDInit(); // Make sure this function is safe to call from this task

        if (payload == "purple")
        {
          for (int i = 0; i < LEDNum; i++)
          {
            LEDSet(i, LEDColorPurple);
          }
        }
        else if (payload == "turquoise")
        {
          for (int i = 0; i < LEDNum; i++)
          {
            LEDSet(i, LEDColorPink);
          }
        }
        LEDShow();
      }
      else
      {
        Serial.print("HTTP GET failed, error code: ");
        Serial.println(httpCode);
      }
      http2.end(); // End the HTTP connection
    }
    else
    {
      // If not connected, attempt to reconnect
      Serial.println("Reconnecting to Wi-Fi...");
      WiFi.disconnect();
      WiFi.reconnect();
      LEDInit();

      for (int i = 0; i < LEDNum; i++)
        LEDSet(i, LEDColorDisconnected);

      LEDShow();

      // Wait a bit before next reconnection attempt
      // vTaskDelay(pdMS_TO_TICKS(5000));
    }

    // Delay to prevent flooding the network with requests
    // vTaskDelay(pdMS_TO_TICKS(5000));
  }

  vTaskDelete(NULL); // Delete this task if it ever breaks out of the loop (which it shouldn't)
}

void TaskUpdateDisplay(void *pvParameters)
{
  for (;;)
  { // Task loop

    // Display the encoder position on the LCD
    char buffer[10];
    sprintf(buffer, "X: %d", encoderPos);
    LCDTextDraw(10, 20, buffer, 1, WHITE, BLACK);

    // Delay for a bit to not update too frequently
    vTaskDelay(pdMS_TO_TICKS(100)); // For example, delay for 100 milliseconds
  }
}