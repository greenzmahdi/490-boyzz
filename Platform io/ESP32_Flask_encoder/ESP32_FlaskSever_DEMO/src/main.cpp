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
const char *ssid = "ssid";
const char *password = "pw";

// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};

// OLED var const
// const int RefreshDelay = 1; // original 5

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"}; // might not need this, depends on design
const char *MenuDroItems[] = {"Sino", "ToAuto"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};

enum MenuState
{
  MAIN_MENU,
  TWO_AXIS,
  THREE_AXIS
};

volatile MenuState currentMenuState = MAIN_MENU;
volatile int menuItemIndex = 0; // Index of the selected menu item

// Forward declarations
void TaskNetwork(void *pvParameters);
void TaskUpdateDisplay(void *pvParameters);

// I2C FUNCTIONS //
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

// BUTTON FUNCTIONS //

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

bool ButtonStatesPrev[] = {false, false, false, false, false};

bool stateButtonCenter = ButtonCenterPressed();
bool stateButtonUp = ButtonUpPressed();
bool stateButtonDown = ButtonDownPressed();
bool stateButtonLeft = ButtonLeftPressed();
bool stateButtonRight = ButtonRightPressed();

void updateButtonStates()
{
  ButtonStatesPrev[0] = ButtonLeftPressed();
  ButtonStatesPrev[1] = ButtonCenterPressed();
  ButtonStatesPrev[2] = ButtonUpPressed();
  ButtonStatesPrev[3] = ButtonDownPressed();
  ButtonStatesPrev[4] = ButtonRightPressed();
}

void handleMenuNavigation()
{
  if (ButtonUpPressed() && !ButtonStatesPrev[2])
  {
    if (currentMenuState == MAIN_MENU)
    {
      menuItemIndex = max(0, menuItemIndex - 1);
    }
  }
  else if (ButtonDownPressed() && !ButtonStatesPrev[3])
  {
    if (currentMenuState == MAIN_MENU)
    {
      menuItemIndex = min(2, menuItemIndex + 1); // Assuming you have 3 menu items (in the case we want to add another option)
    }
  }
  else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
  {
    if (currentMenuState == MAIN_MENU)
    {
      switch (menuItemIndex)
      {
      case 0:
        LCDScreenClear();
        currentMenuState = TWO_AXIS;
        break;
      case 1:
        LCDScreenClear();
        currentMenuState = THREE_AXIS;
        break;
      }
    }
    else
    {
      currentMenuState = MAIN_MENU; // Allow going back to the main menu
      LCDScreenClear();
    }
  }
  // Update previous button states at the end of your button handling logic
  // update button pressed
  ButtonStatesPrev[0] = stateButtonCenter;
  ButtonStatesPrev[1] = stateButtonUp;
  ButtonStatesPrev[2] = stateButtonDown;
  ButtonStatesPrev[3] = stateButtonLeft;
  ButtonStatesPrev[4] = stateButtonRight;
}
// Menu setup
int MotorChannelSelected = 0;
int MotorChannelWatched = -1;

//
const int IdxZ1 = 5;
const int IdxZ2 = 4;
const int IdxZ3 = 3;
const int IdxZ4 = 2;
const int IdxZ5 = 1;
const int IdxZ6 = 0;

// Encoder variables //

volatile int encoderPos = 0;  // Encoder position
volatile int lastEncoded = 0; // Last encoded state

void updateEncoder()
{
  int MSB = digitalRead(PIN_A1);          // Most significant bit (MSB) - pinA
  int LSB = digitalRead(PIN_B1);          // Least significant bit (LSB) - pinB
  int encoded = (MSB << 1) | LSB;         // Converting the 2 pin value to single number
  int sum = (lastEncoded << 2) | encoded; // Adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderPos--;

  lastEncoded = encoded; // Store this value for next time
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

  attachInterrupt(PIN_A1, updateEncoder, CHANGE);
  attachInterrupt(PIN_B1, updateEncoder, CHANGE);

  // Dim LEDs
  FastLED.setBrightness(24);

  // Handle display updates here
  // LCDTextDraw(12, 6, "-COMP491 ESP32 DRO-", 1, 1, 0);

  // Serial.println('\n');
  xTaskCreate(
      TaskNetwork,   // Task function
      "NetworkTask", // Name of the task
      10000,         // Stack size of task
      NULL,          // Parameter of the task
      2,             // Priority of the task
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
      // int outgoingvalue = 123;
      // StaticJsonDocument<200> doc;
      // doc["outgoingvalue"] = outgoingvalue;
      // String jsonstring;
      // serializeJson(doc, jsonstring);

      // If connected, perform HTTP operations

      // HTTPClient http1;
      HTTPClient http2;

      // http1.begin("http://192.168.1.17:5000/getposition");
      // http1.addHeader("Content-Type", "application/json");       // I commented out Randy b/c our flask sever was bugging out due to a (fuc option has no attribute)
      // int httpCode = http1.POST(jsonstring);
      // String payload = http1.getString();
      // http1.end();

      http2.begin("http://192.168.1.17:5000/status"); // Your server URL
      int httpCode2 = http2.GET();

      if (httpCode2 > 0)
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
            LEDSet(i, LEDColorTurquoise);
          }
        }
        LEDShow();
      }
      else
      {
        Serial.print("HTTP GET failed, error code: ");
        Serial.println(httpCode2);
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

      /// Wait a bit before next reconnection attempt
      vTaskDelay(pdMS_TO_TICKS(200));
    }

    // Delay to prevent flooding the network with requests
    // vTaskDelay(pdMS_TO_TICKS(5000));
  }

  vTaskDelete(NULL); // Delete this task if it ever breaks out of the loop (which it shouldn't)
}

// This was the first implementation //

// void TaskUpdateDisplay(void *pvParameters)
// {
//   for (;;)
//   { // Task loop
//     bool stateButtonCenter = ButtonCenterPressed();
//     bool stateButtonUp = ButtonUpPressed();
//     bool stateButtonDown = ButtonDownPressed();
//     bool stateButtonLeft = ButtonLeftPressed();
//     bool stateButtonRight = ButtonRightPressed();

//     static int lastPos = 0; // Last position to check for changes

//     if (encoderPos != lastPos)
//     {
//       Serial.println(encoderPos); // Print the change in position
//       lastPos = encoderPos;       // Update last position
//     }

//     // Display the encoder position on the LCD
//     char buffer[15];

//     // X position
//     LCDRectFill(10, 20, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
//     sprintf(buffer, "X: %d", encoderPos);
//     LCDTextDraw(10, 20, buffer, 1, WHITE, BLACK);

//     //// This is just an example of how it would display on the OLED screen ////

//     // Y position
//     LCDRectFill(10, 35, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
//     sprintf(buffer, "Y: %d", encoderPos);
//     LCDTextDraw(10, 35, buffer, 1, WHITE, BLACK);

//     // Z position
//     LCDRectFill(10, 50, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
//     sprintf(buffer, "Z: %d", encoderPos);
//     LCDTextDraw(10, 50, buffer, 1, WHITE, BLACK);

//     // Delay for a bit to not update too frequently
//     vTaskDelay(pdMS_TO_TICKS(100)); // For example, delay for 100 milliseconds
//   }
// }



void updateDisplayContent()
{
  char buffer[10];

  switch (currentMenuState)
  {
  case MAIN_MENU:
    LCDTextDraw(7, 0, "-COMP491 ESP32 DRO-", 1, WHITE, BLACK);
    for (int i = 0; i < 2; i++)
    {
      sprintf(buffer, "%s %s", (i == menuItemIndex) ? ">" : " ", MenuDroItems[i]);
      LCDTextDraw(0, 16 * (i + 1), buffer, 1, WHITE, BLACK);
    }
    break;
  case TWO_AXIS:
    LCDRectFill(0, 0, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
    sprintf(buffer, "X: %d", encoderPos);
    LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 16, 50, 10, BLACK);    // Fill a rectangle area with BLACK to clear previous number
    sprintf(buffer, "Y: %d", encoderPos); // Placeholder for Y axis
    LCDTextDraw(0, 16, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 50, 50, 10, BLACK);                // Fill a rectangle area with BLACK to clear previous number
    LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
    break;
  case THREE_AXIS:
    LCDRectFill(0, 0, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
    sprintf(buffer, "X: %d", encoderPos);
    LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 16, 50, 10, BLACK);    // Fill a rectangle area with BLACK to clear previous number
    sprintf(buffer, "Y: %d", encoderPos); // Placeholder for Y axis
    LCDTextDraw(0, 16, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 32, 50, 10, BLACK);    // Fill a rectangle area with BLACK to clear previous number
    sprintf(buffer, "Z: %d", encoderPos); // Placeholder for Z axis
    LCDTextDraw(0, 32, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 50, 50, 10, BLACK);                // Fill a rectangle area with BLACK to clear previous number
    LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
    break;
  }
}

void TaskUpdateDisplay(void *pvParameters)
{
  for (;;)
  {
    handleMenuNavigation();
    updateDisplayContent();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}
