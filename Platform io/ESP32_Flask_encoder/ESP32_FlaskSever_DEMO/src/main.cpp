#include <WiFi.h>
#include <HTTPClient.h>
#include <FastLED.h>
// #include <Wire.h>
#include <iostream>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <ArduinoJson.h>

// file imports
#include "pin.h"
#include "led.h"
#include "oled.h"

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

void I2CWriteReg(int address, int pin, bool state)
{
  // This function should write 'state' to 'pin' at 'address' on your I2C expander
  // The implementation details will vary depending on your specific I2C expander chip

  byte dataToWrite;

  // Example: If you need to write a single bit, you'll likely first read the current state
  // of all pins, modify the bit for 'pin' to 'state', and then write back.
  Wire.beginTransmission(address);
  Wire.write(0x00); // Assuming the register you're writing to, often the GPIO register
  if (state)
  {
    dataToWrite |= (1 << pin); // Set the bit for the pin
  }
  else
  {
    dataToWrite &= ~(1 << pin); // Clear the bit for the pin
  }
  Wire.write(dataToWrite);
  Wire.endTransmission();
}

// monitor z pin state //
bool ZPinState(int idx)
{
  // PIN Z1 = 9
  // PIN Z2 = 10
  // PIN Z3 = 11
  // PIN Z4 = 12
  // PIN Z5 = 13

  if ((idx < 9) || (idx > 14))
    return false;

  return !I2CReadReg(0x20, 1, idx);
}

bool pinZ1State()
{
  return ZPinState(9);
}

bool pinZ2State()
{
  return ZPinState(10);
}

bool pinZ3State()
{
  return ZPinState(11);
}

bool pinZ4State()
{
  return ZPinState(12);
}

bool pinZ5State()
{
  return ZPinState(13);
}

bool PinStatePrev[] = {false, true, false, false, false};

bool PinStateZ1 = pinZ1State();
bool PinStateZ2 = pinZ2State();
bool PinStateZ3 = pinZ3State();
bool PinStateZ4 = pinZ4State();
bool PinStateZ5 = pinZ5State();

void updateAllPinZ()
{
  PinStatePrev[0] = pinZ1State();
  PinStatePrev[1] = pinZ2State();
  PinStatePrev[2] = pinZ3State();
  PinStatePrev[3] = pinZ4State();
  PinStatePrev[4] = pinZ5State();
}

bool updateZPinState(int pinAIdx, int pinBIdx, int pinZIdx)
{
  bool stateA = I2CReadReg(0x20, 1, pinAIdx); // Read state of pin A
  bool stateB = I2CReadReg(0x20, 1, pinBIdx); // Read state of pin B

  bool zState = stateA && stateB; // Z is HIGH if both A and B are HIGH

  I2CWriteReg(0x20, pinZIdx, zState); // Update Z pin state

  return zState;
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
  // check if button being pressed is diff from its last prev state aka (true != false)
  if (ButtonUpPressed() && !ButtonStatesPrev[2])
  {
    // ensures curr state is in MAIN_MENU, ensuring it does not go below 0
    if (currentMenuState == MAIN_MENU)
    {
      menuItemIndex = max(0, menuItemIndex - 1);
    }
  }
  // check if button being pressed is diff from its last prev state aka (true != false)
  else if (ButtonDownPressed() && !ButtonStatesPrev[3])
  {
    if (currentMenuState == MAIN_MENU)
    {
      menuItemIndex = min(1, menuItemIndex + 1); // For now we just have two menu options
      // menuItemIndex = min(2, menuItemIndex + 1); // Assuming we want to add 3 menu items (in the case we want to add another option)
    }
  }
  // check if button being pressed is diff from its last prev state aka (true != false)
  else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
  {
    if (currentMenuState == MAIN_MENU)
    {
      // based on the state of our menu option, we update our screen with the correct screen
      // we clear the screen and update display
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
  ButtonStatesPrev[0] = stateButtonCenter;
  ButtonStatesPrev[1] = stateButtonUp;
  ButtonStatesPrev[2] = stateButtonDown;
  ButtonStatesPrev[3] = stateButtonLeft;
  ButtonStatesPrev[4] = stateButtonRight;

  // updateAllPinZ(); // update all
}
// Menu setup
int MotorChannelSelected = 0;
int MotorChannelWatched = -1;

// NOT BEING USED ATM //
const int IdxZ1 = 5;
const int IdxZ2 = 4;
const int IdxZ3 = 3;
const int IdxZ4 = 2;
const int IdxZ5 = 1;
const int IdxZ6 = 0;

// Encoder variables //
struct Encoder
{
  const uint8_t pinA;
  const uint8_t pinB;
  volatile int lastEncoded;
  volatile int position;
};

// volatile int encoderPos = 0; // Encoder position
// volatile int lastEncoded = 0; // Last encoded state

// void updateEncoder()
// {
//   int MSB = digitalRead(PIN_A1);          // Most significant bit (MSB) - pinA
//   int LSB = digitalRead(PIN_B1);          // Least significant bit (LSB) - pinB
//   int encoded = (MSB << 1) | LSB;         // Converting the 2 pin value to single number
//   int sum = (lastEncoded << 2) | encoded; // Adding it to the previous encoded value

//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
//     encoderPos++;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
//     encoderPos--;

//   lastEncoded = encoded; // Store this value for next time
// }

void updateEncoderPos(Encoder *encoder)
{
  int MSB = digitalRead(encoder->pinA);            // Most significant bit (MSB) - pinA
  int LSB = digitalRead(encoder->pinB);            // Least significant bit (LSB) - pinB
  int encoded = (MSB << 1) | LSB;                  // Converting the 2 pin value to single number
  int sum = (encoder->lastEncoded << 2) | encoded; // Adding it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoder->position++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoder->position--;

  encoder->lastEncoded = encoded; // Store this value for next time
}

Encoder encoder1 = {PIN_A1, PIN_B1, 0, 0};
Encoder encoder2 = {PIN_A2, PIN_B2, 0, 0};
Encoder encoder3 = {PIN_A3, PIN_B3, 0, 0};
Encoder encoder4 = {PIN_A4, PIN_B4, 0, 0};
Encoder encoder5 = {PIN_A5, PIN_B5, 0, 0};
Encoder encoder6 = {PIN_A6, PIN_B6, 0, 0};

void IRAM_ATTR handleEncoder1Interrupt()
{
  updateEncoderPos(&encoder1);
}
void IRAM_ATTR handleEncoder2Interrupt()
{
  updateEncoderPos(&encoder2);
}

void IRAM_ATTR handleEncoder3Interrupt()
{ // might not need this one
  updateEncoderPos(&encoder3);
}

void IRAM_ATTR handleEncoder4Interrupt()
{
  updateEncoderPos(&encoder4);
}

void IRAM_ATTR handleEncoder5Interrupt()
{
  updateEncoderPos(&encoder5);
}

void IRAM_ATTR handleEncoder6Interrupt()
{
  updateEncoderPos(&encoder6);
}

void updateAllZPins() {
    // Example calls, assuming pinA1Index and pinB1Index are the indexes for A1 and B1 on the expander
    updateZPinState(encoder1.pinA, encoder1.pinB, 9); // For Z1
    updateZPinState(encoder2.pinA, encoder2.pinB, 10); // For Z2
    updateZPinState(encoder3.pinA, encoder3.pinB, 11); // For Z3
    updateZPinState(encoder4.pinA, encoder4.pinB, 12); // For Z4
    updateZPinState(encoder5.pinA, encoder5.pinB, 13); // For Z5
    updateZPinState(encoder6.pinA, encoder6.pinB, 14); // For Z6
    // Repeat for other Z pins and their corresponding A/B pins
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

  // Monitor pin setup //

  attachInterrupt(digitalPinToInterrupt(encoder1.pinA), handleEncoder1Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder1.pinB), handleEncoder1Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder2.pinA), handleEncoder2Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder2.pinB), handleEncoder2Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder3.pinA), handleEncoder3Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder3.pinB), handleEncoder3Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder4.pinA), handleEncoder4Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder4.pinB), handleEncoder4Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder5.pinA), handleEncoder5Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder5.pinB), handleEncoder5Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder6.pinA), handleEncoder6Interrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoder6.pinB), handleEncoder6Interrupt, CHANGE);

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
      //Randy code
      String outgoingvalue = "123";
      
      HTTPClient http;
      http.begin("http://192.168.1.132:5000/getposition");
      http.addHeader("Content-Type", "application/json");
      
      StaticJsonDocument<200> doc;
      doc["value1"] = encoder1.position;
      doc["value2"] = encoder2.position;
      String code;
      serializeJson(doc, code);
      int httpResponseCode = http.POST(code);

      /*int code = encoder1.position;
      std::string strNum = std::to_string(code);
      const char* charArray = strNum.c_str();
      int httpResponseCode = http.POST(charArray);*/


      if (httpResponseCode > 0)
      {
        String response = http.getString();
        Serial.print("Ok");
        delay(5000);
      }
      else{
        Serial.print("Wrong");
        delay(10000);
      }
      http.end();
      
      HTTPClient http2;
      http2.begin("http://rhcsun.pythonanywhere.com/status"); // Your server URL
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

void updateDisplayContent()
{
  char buffer[10];

  switch (currentMenuState)
  {
  case MAIN_MENU:
    LCDTextDraw(7, 0, " COMP491 ESP32 DRO ", 1, WHITE, BLACK);
    for (int i = 0; i < 2; i++)
    {
      sprintf(buffer, "%s %s", (i == menuItemIndex) ? ">" : " ", MenuDroItems[i]);
      LCDTextDraw(0, 16 * (i + 1), buffer, 1, WHITE, BLACK);
    }
    break;
  case TWO_AXIS:
    LCDRectFill(0, 0, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
    // sprintf(buffer, "X: %d", encoderPos); // original implementation
    sprintf(buffer, "X: %d", encoder1.position);
    LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 16, 50, 10, BLACK);
    sprintf(buffer, "Y: %d", encoder2.position);
    LCDTextDraw(0, 16, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 50, 50, 10, BLACK);
    LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
    break;
  case THREE_AXIS:
    LCDRectFill(0, 0, 50, 10, BLACK);
    sprintf(buffer, "X: %d", encoder4.position);
    LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 16, 50, 10, BLACK);
    sprintf(buffer, "Y: %d", encoder5.position);
    LCDTextDraw(0, 16, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 32, 50, 10, BLACK);
    sprintf(buffer, "Z: %d", encoder6.position);
    LCDTextDraw(0, 32, buffer, 1, WHITE, BLACK);

    LCDRectFill(0, 50, 50, 10, BLACK);
    LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
    break;
  }
}

void TaskUpdateDisplay(void *pvParameters)
{
  for (;;)
  {
    updateAllZPins();
    handleMenuNavigation();
    updateDisplayContent();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}
