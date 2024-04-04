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
 
// file imports
#include "I2C.h"
#include "pin.h"
#include "led.h"
#include "oled.h"
#include "encoder.h"
 
//Mode selector variable
bool isInchMode = true;
 
//Inch and milimeter factors
int factor_inch = 2;
float factor_mm = 0.5;
 
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
 
// Initializing encoders attributes and setting their start (refer to encoder struct to see all parameters)
Encoder encoder1 = {PIN_A1, PIN_B1, 0, 0, 0, 0, {0}, 1};
Encoder encoder2 = {PIN_A2, PIN_B2, 0, 0, 0, 0, {0}, 1};
Encoder encoder3 = {PIN_A3, PIN_B3, 0, 0, 0, 0, {0}, 1};
Encoder encoder4 = {PIN_A4, PIN_B4, 0, 0, 0, 0, {0}, 1};
Encoder encoder5 = {PIN_A5, PIN_B5, 0, 0, 0, 0, {0}, 1};
Encoder encoder6 = {PIN_A6, PIN_B6, 0, 0, 0, 0, {0}, 1};
 
void IRAM_ATTR handleEncoder1Interrupt() { updateEncoder(&encoder1); }
void IRAM_ATTR handleEncoder2Interrupt() { updateEncoder(&encoder2); }
void IRAM_ATTR handleEncoder3Interrupt() { updateEncoder(&encoder3); }
void IRAM_ATTR handleEncoder4Interrupt() { updateEncoder(&encoder4); }
void IRAM_ATTR handleEncoder5Interrupt() { updateEncoder(&encoder5); }
void IRAM_ATTR handleEncoder6Interrupt() { updateEncoder(&encoder6); }
 
// void updateAllZPins()
// {
//   // Example calls, assuming pinA1Index and pinB1Index are the indexes for A1 and B1 on the expander
//   updateZPinState(encoder1.pinA, encoder1.pinB, 9);  // For Z1
//   updateZPinState(encoder2.pinA, encoder2.pinB, 10); // For Z2
//   updateZPinState(encoder3.pinA, encoder3.pinB, 11); // For Z3
//   updateZPinState(encoder4.pinA, encoder4.pinB, 12); // For Z4
//   updateZPinState(encoder5.pinA, encoder5.pinB, 13); // For Z5
//   updateZPinState(encoder6.pinA, encoder6.pinB, 14); // For Z6
//                                                      // Repeat for other Z pins and their corresponding A/B pins
// }
 
const char *h_ssid = "491-DRO-Boyyz";
const char *h_password = "123456789";
 
const int ledPin = 12;  // The GPIO pin connected to your LED strip
const int numLeds = 12; // Number of LEDs in your strip
 
CRGB leds[numLeds];
AsyncWebServer server(80);
 
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>DRO Interface</title>
<style>
  #indicator {
    font-size: 12px;
    margin-left: 10px;
  }
 
  h1 {
    text-align: center;
  }
  body {
    font-family: Arial, sans-serif;
    background: #f0f0f0;
  }
  .dro-container {
    width: 600px;
    margin: 0 auto;
    background: #ddd;
    padding: 20px;
    border-radius: 10px;
  }
  .readout {
    font-family: 'Digital', sans-serif;
    background: #000;
    color: #00ff00;
    padding: 10px;
    margin-bottom: 5px;
  }
  .calculator-grid {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 10px;
    margin-top: 20px;
  }
  .calculator-button {
    padding: 20px;
    background: #fff;
    border: 1px solid #ccc;
    border-radius: 5px;
    text-align: center;
    cursor: pointer;
  }
  .calculator-button:hover {
    background: #e9e9e9;
  }
  /* More styles can be added as per requirement */
</style>
<script>
  function toggleMode() {
    fetch("toggle-mode")
    .then(response=> response.text())
    .then(data => {
      document.getElementById("modeIndicator").innerText = data;
      updatePosition();
    })
    .catch(console.error);
  }
 
  function updatePosition() {
    fetch("/poss")
    .then(response => response.text())
    .then(data => document.getElementById("poss").innerText = data)
    .catch(console.error);
  }
</script>
</head>
<body>
<h1>491 ESP32 DRO Boyyz</h1>
</hr>
<div class="dro-container">
  <div class="readout" id="x-readout">MODE: ABS</div>
  <div class="readout" id="x-readout">X: <span id="position"></span></div>
  <div class="readout" id="y-readout">Y: <span id="position2"></span></div>
  <div class="readout" id="z-readout">Z: <span id="position3"></span></div>
</div>
 
<div class="dro-container">
  <!-- This is for selecting an Axis and for Zeroing out an axis -->
  <p>Zero out / Select Buttons</p>
  <button onclick="resetPosition('x')">Xo</button>
  <button onclick="resetPosition('y')">Yo</button>
  <button onclick="resetPosition('z')">Zo</button>
  <button onclick="resetPosition('select_x')">Select X</button>
  <button onclick="resetPosition('select_y')">Select Y</button>
  <button onclick="resetPosition('select_z')">Select Z</button>
</div>
 
 
<!-- NOTE Mahdi this is how I made the grid, note how I used class ="calculator-grid", I set the grid style -->
<div class="dro-container">
  <p>Calculator Buttons</p>
  <div class ="calculator-grid"> 
    <button>1/2</button>
    <button id="toggleButton" onclick="toggleMode()">INCH/MM</button>
    <span id="modeIndicator">INCH</span>
    <p>Position: <span id="poss">0</span></p>
    <button>9</button>
    <button>8</button>
    <button>7</button>
    <button>6</button>
    <button>5</button>
    <button>4</button>
    <button>3</button>
    <button>2</button>
    <button>1</button>
    <button>.</button>
    <button>0</button>
    <button>+/-</button>
  </div>
</div>
 
<div class="dro-container">
  <!-- These need to be Implemented still, Randy I created the button dor ABS/INC juyt join the logic using the 2 buttons I already made or whatever is easier for you!-->
  <p>Zero all out / ABS/INC mode / Calculate / Enter Buttons</p>
  <button>ABS/INC</button>
  <button>XYZo</button>
  <button>CA</button>
  <button>ENT</button>
  <button onclick="resetPosition('xInc')">Abs X</button>
  <button onclick="resetPosition('xAbs')"> Inc</button>
</div>
 
<div class="dro-container">
  <!-- This is for our function key layout-->
  <p>Function Buttons</p>
  <button>F1</button>
  <button>F2</button>
  <button>F3</button>
  <button>F4</button>
  <button>F5</button>
  <button>F6</button>
</div>
 
<div class="dro-container">
  <!-- This is for the GRID buttons-->
  <p>Grid Buttons</p>
  <button>option1</button>
  <button>option2</button>
  <button>option3</buton>
  <button>option4</button>
</div>
 
<div class="dro-container">
  <!-- Arrow Keys-->
  <p>Arrow Buttons</p>
  <button>↑</button>
  <button>→</button>
  <button>↓</buton>
  <button>←</button>
</div>
 
 
<script> 
  updatePosition();
  
  function updatePositions() {
    fetch("/position")
      .then(response => response.text())
      .then(data => document.getElementById("position").innerText = data)
      .catch(console.error);
 
    fetch("/position2")
      .then(response => response.text())
      .then(data => document.getElementById("position2").innerText = data)
      .catch(console.error);
 
    fetch("/position3")
      .then(response => response.text())
      .then(data => document.getElementById("position3").innerText = data)
      .catch(console.error);
  }
 
  function resetPosition(axis) {
  fetch("/reset/" + axis)
    .then(response => {
      if (response.ok) {
        console.log(axis.toUpperCase() + " position reset"); // print message 
        updatePositions(); // Refresh the positions immediately
      }
    })
    .catch(console.error);
}
 
//   function togglePosition(position) {
//   fetch("/switch/" + position)
//     .then(response => {
//       if (response.ok) {
//         console.log(axis.toUpperCase() + " position reset"); // print message 
//         updatePositions(); // Refresh the positions immediately
//       }
//     })
//     .catch(console.error);
// }
 
 
 
  setInterval(updatePositions, 50);   // Call updatePositions() every 1000ms (1 second) but right now it is 50ms so stupid fast 
</script>
</body>
</html>
)rawliteral";
 
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
 
  // Setting up the ESP32 as an Access Point //
  WiFi.softAP(h_ssid, h_password);
  Serial.println("Access Point Started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
 
  // Setting up Routes
  // Route for root web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){ 
    request->send_P(200, "text/html", index_html); });
 
  // New route to get the current position of encoder1
  server.on("/position", HTTP_GET, [](AsyncWebServerRequest *request){
    char temp[100];
    snprintf(temp, 100, "%d", encoder1.position); // Assuming encoder1.position is an int
    request->send(200, "text/plain", temp); });
 
  server.on("/position2", HTTP_GET, [](AsyncWebServerRequest *request){
    char temp[100];
    snprintf(temp, 100, "%d", encoder2.position); // Assuming encoder1.position is an int
    request->send(200, "text/plain", temp); });
 
  server.on("/position3", HTTP_GET, [](AsyncWebServerRequest *request){
    char temp[100];
    snprintf(temp, 100, "%d", encoder3.position); // Assuming encoder1.position is an int
    request->send(200, "text/plain", temp); });
 
  server.on("/poss", HTTP_GET, [](AsyncWebServerRequest *request) {
    float position = encoder1.position * (isInchMode ? factor_inch : factor_mm);
    char response[100];
    snprintf(response, 100, "%.2f", position);
    request->send(200, "text/plain", response);
 
  });
  server.on("/toggle-mode", HTTP_GET, [](AsyncWebServerRequest *request){
    isInchMode = !isInchMode;
    request->send(200, "text/plain", isInchMode ? "INCH" : "MM");
  });
 
  // //Millimeter request
  //   server.on("/milli", HTTP_GET, [](AsyncWebServerRequest *request) {
  //     //what is the multiplicative factor?
  //     float position_mm = encoder1.position * factor;
 
  //     char temp[100];
  //     snprintf(temp, sizeof(temp), "%.2f", position_mm);
  //     request->send(200, "text/plain", temp); });
 
  // //Mid-point calculation
  // server.on("/half", HTTP_GET, [](AsyncWebServerRequest *request) {
  //   char temp[100];
  //   float position_half = encoder1.position/2;
  //   snprintf(temp, sizeof(temp), "%.2f",)
  //   request->send(200, "text/plain", temp);
  // });
 
  // Routes to toggle LED colors
  server.on("/turquoise", HTTP_GET, [](AsyncWebServerRequest *request){
    for(int i = 0; i < numLeds; i++) leds[i] = CRGB::Turquoise;
      FastLED.show();
      request->send(200, "text/plain", "LEDs set to Turquoise"); });
 
  server.on("/purple", HTTP_GET, [](AsyncWebServerRequest *request){
    for(int i = 0; i < numLeds; i++) leds[i] = CRGB::Purple;
      FastLED.show();
      request->send(200, "text/plain", "LEDs set to Purple"); });
 
  // Routes to reset Axis position
  server.on("/reset/x", HTTP_GET, [](AsyncWebServerRequest *request){
  encoder1.positionInc = encoder1.position;
  encoder1.position = 0; // Reset X position
  request->send(200, "text/plain", "X position reset"); });
 
  server.on("/reset/xInc", HTTP_GET, [](AsyncWebServerRequest *request){
  // encoder2.position = 0; // Reset Y position
 
  encoder1.position += encoder1.positionInc;
 
  request->send(200, "text/plain", "Y position reset"); });
 
  server.on("/reset/xAbs", HTTP_GET, [](AsyncWebServerRequest *request){
  // encoder2.position = 0; // Reset Y position
  encoder1.position += encoder1.positionInc;
  request->send(200, "text/plain", "Y position reset"); });
 
  //
 
  server.on("/reset/y", HTTP_GET, [](AsyncWebServerRequest *request){
  encoder2.position = 0; // Reset Y position
  request->send(200, "text/plain", "Y position reset"); });
 
  //
 
  server.on("/reset/z", HTTP_GET, [](AsyncWebServerRequest *request){
  encoder3.position = 0; // Assuming encoder3 is for Z, reset Z position
  request->send(200, "text/plain", "Z position reset"); });
 
  server.on("/switch/abs", HTTP_GET, [](AsyncWebServerRequest *request){
  char temp[100];
  snprintf(temp, 100, "%d", encoder1.position); // Assuming encoder1.position is an int
  request->send(200, "text/plain", temp); });
 
  server.on("/switch/inc", HTTP_GET, [](AsyncWebServerRequest *request){
  encoder1.positionInc = encoder1.position; // Assuming encoder3 is for Z, reset Z position
  encoder1.position += encoder1.positionInc;
  request->send(200, "text/plain", "Z position reset"); });
 
  // Axis Selector buttons 
 
 
  server.begin();
 
  // Monitor pin setup //
  attachInterrupt(digitalPinToInterrupt(encoder1.pinA), handleEncoder1Interrupt, CHANGE); // I need to remove the lambda and include the w/ name of interrupt to ensure we are using the correct one
 
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
    // updateAllZPins();
    handleMenuNavigation();
    updateDisplayContent();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}