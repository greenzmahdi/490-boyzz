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

// Mode selector variable
bool isInchMode = true;

// Inch and milimeter factors
const float factor_mm = 0.01;        // 0.01 mm per pulse
const float factor_inch = 0.0003937; // Accurate conversion to maintain equivalence

#define SCREEN_WIDTH 128 // OLED display width
#define CHAR_WIDTH 6     // Width of each character in pixels

// Function to format position to fixed decimal places
String formatPosition(float pulses, bool isInchMode)
{
  float convertedValue;
  char formattedOutput[20]; // Buffer to hold the formatted string

  if (isInchMode)
  {
    convertedValue = pulses * factor_inch;
    snprintf(formattedOutput, sizeof(formattedOutput), "%7.4f", convertedValue); // Ensures 4 decimal places
  }
  else
  {
    convertedValue = pulses * factor_mm;
    snprintf(formattedOutput, sizeof(formattedOutput), "%6.3f", convertedValue); // Ensures 3 decimal places
  }

  return String(formattedOutput); // Convert buffer to Arduino String object for easy use
}

// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};

// OLED var const
// const int RefreshDelay = 1; // original 5

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"}; // might not need this, depends on design
const char *MenuDroItems[] = {"2-Axis", "3-Axis"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};

enum MenuState
{
  MAIN_MENU,
  TWO_AXIS,
  THREE_AXIS,
  SHAPE_CREATION
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

// void handleMenuNavigation()
// {
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   if (ButtonUpPressed() && !ButtonStatesPrev[2])
//   {
//     // ensures curr state is in MAIN_MENU, ensuring it does not go below 0
//     if (currentMenuState == MAIN_MENU)
//     {
//       menuItemIndex = max(0, menuItemIndex - 1);
//     }
//   }
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   else if (ButtonDownPressed() && !ButtonStatesPrev[3])
//   {
//     if (currentMenuState == MAIN_MENU)
//     {
//       menuItemIndex = min(1, menuItemIndex + 1); // For now we just have two menu options
//       // menuItemIndex = min(2, menuItemIndex + 1); // Assuming we want to add 3 menu items (in the case we want to add another option)
//     }
//   }
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
//   {
//     if (currentMenuState == MAIN_MENU)
//     {
//       // based on the state of our menu option, we update our screen with the correct screen
//       // we clear the screen and update display
//       switch (menuItemIndex)
//       {
//       case 0:
//         LCDScreenClear();
//         currentMenuState = TWO_AXIS;
//         break;
//       case 1:
//         LCDScreenClear();
//         currentMenuState = THREE_AXIS;
//         break;
//       }
//     }
//     else
//     {
//       currentMenuState = MAIN_MENU; // Allow going back to the main menu
//       LCDScreenClear();
//     }
//   }
//   // Update previous button states at the end of your button handling logic
//   ButtonStatesPrev[0] = stateButtonCenter;
//   ButtonStatesPrev[1] = stateButtonUp;
//   ButtonStatesPrev[2] = stateButtonDown;
//   ButtonStatesPrev[3] = stateButtonLeft;
//   ButtonStatesPrev[4] = stateButtonRight;

//   // updateAllPinZ(); // update all
// }






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

// Encoder encoder4 = {PIN_A4, PIN_B4, 0, 0, 0, 0, {0}, 1};
// Encoder encoder5 = {PIN_A5, PIN_B5, 0, 0, 0, 0, {0}, 1};
// Encoder encoder6 = {PIN_A6, PIN_B6, 0, 0, 0, 0, {0}, 1};

// void IRAM_ATTR handleEncoder1Interrupt() { updateEncoder(&encoder1); }
// void IRAM_ATTR handleEncoder2Interrupt() { updateEncoder(&encoder2); }
// void IRAM_ATTR handleEncoder3Interrupt() { updateEncoder(&encoder3); }
// void IRAM_ATTR handleEncoder4Interrupt() { updateEncoder(&encoder4); }
// void IRAM_ATTR handleEncoder5Interrupt() { updateEncoder(&encoder5); }
// void IRAM_ATTR handleEncoder6Interrupt() { updateEncoder(&encoder6); }

bool isABSMode = true; // Start in ABS mode


int setupEncoderValuesABs[] = {};

struct Point {
    int x, y, z; // Include z if you plan to extend to 3D shapes

    Point(int px, int py, int pz = 0) : x(px), y(py), z(pz) {} // Constructor for initialization
};

struct CoordinatePlane {
    std::vector<Point> shapePoints; // Store points for each shape in the plane
    int encoderValueABS[3];    // Store ABS values for X, Y, Z
    int encoderValueINC[3];    // Store INC values for X, Y, Z
};

// Store up to 12 coordinate planes 
CoordinatePlane planes[12];


int currentPlaneIndex = 0; // Keep track of the current plane index

void selectPlane(int index) {
    if (index >= 0 && index < 12) {
        currentPlaneIndex = index;
        // Update display or other state changes needed when switching planes (needs to be implemented)
        // updateDisplayContent();
    }
}

void nextPlane() {
    selectPlane((currentPlaneIndex + 1) % 12);
    Serial.print("Next Plane: "); Serial.println(currentPlaneIndex + 1);
}

void previousPlane() {
    selectPlane((currentPlaneIndex + 11) % 12);
    Serial.print("Previous Plane: "); Serial.println(currentPlaneIndex + 1);
}


void addPointToCurrentPlane(int planeIndex, int x, int y, int z = 0) {
    if (planeIndex >= 0 && planeIndex < 12) {
        planes[planeIndex].shapePoints.emplace_back(x, y, z);
        // updateDisplayContent();  // Assuming you have a method to update display
    }
}

void removeLastPointFromCurrentPlane(int planeIndex) {
    if (planeIndex >= 0 && planeIndex < 12 && !planes[planeIndex].shapePoints.empty()) {
        planes[planeIndex].shapePoints.pop_back();
        // updateDisplayContent();
    }
}

void displayCurrentPoints(int planeIndex) {
    if (planeIndex >= 0 && planeIndex < 12) {
        auto& points = planes[planeIndex].shapePoints;
        for (size_t i = 0; i < points.size(); ++i) {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "Point %zu: (%d, %d, %d)", i + 1, points[i].x, points[i].y, points[i].z);
            LCDTextDraw(0, i * 16, buffer, 1, WHITE, BLACK);  // Adjust positioning as needed
        }
    }
}

void clearPointsInPlane(int planeIndex) {
    if (planeIndex >= 0 && planeIndex < 12) {
        planes[planeIndex].shapePoints.clear();
        // updateDisplayContent();
    }
}



int X_last_ABS = 0;
int Y_last_ABS = 0;
int Z_last_ABS = 0;

int X_last_INC = 0;
int Y_last_INC = 0;
int Z_last_INC = 0;

int currentSelecteAxis = 0;
void toggleMode()
{
  isABSMode = !isABSMode;
}
void toggleMeasurementMode()
{
  isInchMode = !isInchMode;
}

void resetEncoderValue(int encoderIndex)
{
  if (encoderIndex < 0 || encoderIndex >= 3) {
        Serial.println("Error: Encoder index out of range");
        return; // Add error handling or user feedback
    }
  // Placeholder for resetting encoder value.
  // if we zero out a value we need to store the last position before zeroing out to calculate the difference of (encoder.position - lastValVisited)
  if (isABSMode)
  {
    // reset value back to 0
    planes[currentPlaneIndex].encoderValueABS[encoderIndex] = 0;
    

    // store last coordinate (value) listed
    if (encoderIndex == 0)
    {
      X_last_ABS = encoder1.position;
    }
    else if (encoderIndex == 1)
    {
      Y_last_ABS = encoder2.position;
    }
    else if (encoderIndex == 2)
    {
      Z_last_ABS = encoder3.position;
    }
  }
  else
  {
    // reset value back to 0
    planes[currentPlaneIndex].encoderValueINC[encoderIndex] = 0;

    // store last coordinate (value) listed
    if (encoderIndex == 0)
    {
      X_last_INC = encoder1.position;
    }
    else if (encoderIndex == 1)
    {
      Y_last_INC = encoder2.position;
    }
    else if (encoderIndex == 2)
    {
      Z_last_INC = encoder3.position;
    }
  }
  // Consider adding logic to update the display or take other actions.
}

// Helper function to format and display axis values based on current settings
void displayAxisValues(int axis, int yPosition) {
    char buffer[40];
    int xOffset;
    int position = isABSMode ? planes[currentPlaneIndex].encoderValueABS[axis] : planes[currentPlaneIndex].encoderValueINC[axis];
    snprintf(buffer, sizeof(buffer), "%c: %s", 'X' + axis, formatPosition(position, isInchMode).c_str());
    xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
    LCDTextDraw(xOffset, yPosition, buffer, 1, WHITE, BLACK);
}

void handleMenuNavigation() {
    // Navigate menu options in the main menu
    if (currentMenuState == MAIN_MENU) {
        if (ButtonUpPressed() && !ButtonStatesPrev[2]) {
            menuItemIndex = max(0, menuItemIndex - 1);
        } else if (ButtonDownPressed() && !ButtonStatesPrev[3]) {
            menuItemIndex = min(1, menuItemIndex + 1);  // Only two options
        } else if (ButtonCenterPressed() && !ButtonStatesPrev[1]) {
            switch (menuItemIndex) {
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
    }
    // Handle plane navigation in the TWO_AXIS state
    else if (currentMenuState == TWO_AXIS) {
        if (ButtonLeftPressed() && !ButtonStatesPrev[0]) {
            previousPlane();
            // updateDisplayContent();  // Show updated plane information
        } else if (ButtonRightPressed() && !ButtonStatesPrev[4]) {
            nextPlane();
            // updateDisplayContent();  // Show updated plane information
        } else if (ButtonCenterPressed() && !ButtonStatesPrev[1]) {
            currentMenuState = MAIN_MENU;  // Return to main menu on center button
            LCDScreenClear();
        }
    }
    // Handle plane navigation in the THREE_AXIS state
    else if (currentMenuState == THREE_AXIS) {
        if (ButtonLeftPressed() && !ButtonStatesPrev[0]) {
            previousPlane();
            // updateDisplayContent();  // Show updated plane information
        } else if (ButtonRightPressed() && !ButtonStatesPrev[4]) {
            nextPlane();
            // updateDisplayContent();  // Show updated plane information
        } else if (ButtonCenterPressed() && !ButtonStatesPrev[1]) {
            currentMenuState = MAIN_MENU;  // Return to main menu on center button
            LCDScreenClear();
        }
    }
    // Update the stored state of buttons after handling logic
    updateButtonStates();
}


/*
Right now I just am getting the current value of the given axis, but I still need to:
  - link the button to get current value of coordinate posotion o ngiven axis (mayve have individual varibales for each value)
     but that would depend on the manual whether it allows for multiple axis to be selected at once
     
  - We still need to add the path for each sever.on ...... 
  - add css to highlight the selected axis and deselect if presses again */
// void selectGivenAxis(int encoderIndex){
//   if (isABSMode)
//   {
//     currentSelecteAxis = encoderValueABS[encoderIndex];
//   } else {
//     currentSelecteAxis = encoderValueINC[encoderIndex];
//   }
// }

/* I need to create unique X_last_ABS and X_Last_INC to be unqie for each of the 12 Coordinate planes so that we dont duplicate positions or overide measurements from curr pos */
// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder1Interrupt()
{
  handleEncoderInterrupt(&encoder1); // Assume encoder1 is an instance of Encoder
  // for all axis on ABS Mode
  planes[currentPlaneIndex].encoderValueABS[0] = encoder1.position - X_last_ABS;
  planes[currentPlaneIndex].encoderValueINC[0] = encoder1.position - X_last_INC;

  // // Optionally add point on certain condition but we might not needs this at all 
  //   if (some_condition_met) {
  //       addPointToPlane(currentPlaneIndex, encoder1.position, encoder2.position);
  //   }
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder2Interrupt()
{
  handleEncoderInterrupt(&encoder2); // Assume encoder1 is an instance of Encoder
  planes[currentPlaneIndex].encoderValueABS[1] = encoder2.position - Y_last_ABS;
  planes[currentPlaneIndex].encoderValueINC[1] = encoder2.position - X_last_INC;
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder3Interrupt()
{
  handleEncoderInterrupt(&encoder3); // Assume encoder1 is an instance of Encoder
  planes[currentPlaneIndex].encoderValueABS[2] = encoder3.position - Z_last_ABS;
  planes[currentPlaneIndex].encoderValueINC[2] = encoder3.position - X_last_INC;
}

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
  body {
    font-family: Arial, sans-serif;
    background: #f0f0f0;
    margin: 0;
    padding: 0;
    display: flex;
    justify-content: center;
    height: 100vh;
    align-items: center;
  }

  .flex-container {
    display: flex;
    width: 90%;
    height: 90%;
    align-items: stretch;
  }

  .left-column,
  .right-column {
    display: flex;
    flex-direction: column;
    justify-content: space-between;
  }

  .left-column {
    flex: 3;
  }

  .right-column {
    flex: 2;
    display: flex;
    flex-direction: column;
    justify-content: space-between;
  }

  .dro-container {
    background: #ddd;
    padding: 10px;
    border-radius: 5px;
    margin-bottom: 5px;
  }

  .readout {
  font-family: 'Digital', sans-serif;
  background: #000;
  color: #00ff00;
  padding: 15px; /* Increased padding */
  font-size: 1.2em; /* Larger font size */
  margin-bottom: 10px; /* Adjusted margin */
  border: 1px solid #333; /* Subtle border */
}

  .calculator-grid{
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    gap: 10px;
  }

  .zeroSelect-grid {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: px;
  }

  button {
  padding: 15px;
  background: #f5f5f5; /* Lighter background */
  border: 1px solid #bbb; /* Softer border color */
  border-radius: 5px;
  text-align: center;
  cursor: pointer;
  transition: background-color 0.3s; /* Smooth transition for hover effect */
}

button:hover {
  background: #e0e0e0; /* Subtle hover effect */
}

  .wide {
    grid-column: span 3;
  }
#calculatorModeBtn {
    /* Add default styles for the button */
    padding: 10px;
    border: none;
    cursor: pointer;
}

.calculator-mode-enabled {
    /* Add styles for the button when calculator mode is enabled */
    background-color: green;
    color: white;
}

.calculator-mode-disabled {
    /* Add styles for the button when calculator mode is disabled */
    background-color: gray;
    color: white;
}
</style>
</head>
<body>

<div class="flex-container">
  <div class="left-column">
    <!-- Container 1: Readout displays (X, Y, Z) -->
    <div class="flex-container">
      <div class="left-column">
        <div class="dro-container">
          <div class="readout" id="modeIndicator">ABS</div>

          <div class="readout modeDisplay" id="mode-readout">
            <span id="modeMeasureIndicator">INCH</span>
          </div>
          <div class="readout" id="currentPlane">Plane: 1</div>

          
          <div class="readout" id="x-readout">X: <span id="position"></span></div>
          <div class="readout" id="y-readout">Y: <span id="position2"></span></div>
          <div class="readout" id="z-readout">Z: <span id="position3"></span></div>
          <span id="calctemp"></span>
          <button id="plusBtn" class="operator" onclick="addOperation('+')">PLUS</button>
          <button id="multiplyBtn" class="operator" onclick="addOperation('*')">MULTI</button>
          <button id="subtractBtn" class="operator" onclick="addOperation('-')">SUB</button>
          <button id="divideBtn" class="operator" onclick="addOperation('/')">DIV</button>
          <button onclick="calculate()">EQUAL</button>
          
        </div>
      </div>

      <div class="right-column">
        <div class="dro-container">
          <div class="zeroSelect-grid">
            <button onclick="resetPosition('x')">Xo</button>
            <button onclick="resetPosition('select_x')">Select X</button>
            <button onclick="resetPosition('y')">Yo</button>
            <button onclick="resetPosition('select_y')">Select Y</button>
            <button onclick="resetPosition('z')">Zo</button>        
            <button onclick="resetPosition('select_z')">Select Z</button>
          </div>
        </div>

      </div>
  

    </div>
    <!-- Container 5: Function keys -->
    <div class="dro-container">
      <button>F1</button>
      <button>F2</button>
      <button>F3</button>
      <button>F4</button>
      <button>F5</button>
      <button>F6</button>
    </div>

    <div class="dro-container">
      <button>Process Holes Circle </button>
      <button>Process Holes Line</button>
      <button>R Cut</button>
      <button>Process Slope</button>
    
    </div>

    <!-- Container 6: Arrow keys -->
    <div class="dro-container">
      <button>↑</button>
      <button>→</button>
      <button>↓</button>
      <button>←</button>
    </div>
  </div>

  <div class="right-column">
    <!-- Container 2: Axis selection and zeroing -->
    
    <!-- Container 3: Calculator grid -->
    <div class="dro-container wide">
      <div class="calculator-grid">
        <button>1/2</button>
        <button id="toggleButton" onclick="toggleMeasureMode()">INCH/MM</button>
        <!-- <span id="modeMeasureIndicator">INCH</span> -->
        <!-- <p>Position: <span id="poss">0</span></p> -->

        <button id="calculatorModeBtn" onclick="toggleCalculatorMode()">Calculator Mode</button>
        <button onclick="addNumber(9)">9</button>
        <button onclick="addNumber(8)">8</button>
        <button onclick="addNumber(7)">7</button>
        <button onclick="addNumber(6)">6</button>
        <button onclick="addNumber(5)">5</button>
        <button onclick="addNumber(4)">4</button>
        <button onclick="addNumber(3)">3</button>
        <button onclick="addNumber(2)">2</button>
        <button onclick="addNumber(1)">1</button>
        <button onclick="addDecimal()">.</button>
        <button onclick="addNumber(0)">0</button>
        <button>+/-</button>
      </div>
    </div>

    <!-- Container 4: Additional control buttons -->
    <div class="dro-container wide">
      <div class="zeroSelect-grid">
        <button onclick="toggleMode()">Toggle ABS/INC</button>
        <button id="zeroAllButton" onclick="zeroAllAxis()">XYZo</button>
        <button>CA</button>
        <button>ENT</button>
        <button onclick="resetPosition('xInc')">Abs X</button>
        <button onclick="resetPosition('xAbs')">Inc</button>
      </div>
    </div>
  </div>
</div>

<script>
var currentOperation = null;
var tempNumber = "";
var currentValue = 0;
var calculatorMode = false;

function toggleCalculatorMode() {
    calculatorMode = !calculatorMode;
    updateCalculatorModeButton(); // Update button style
    updateOperatorButtons(); // Update operator buttons based on the calculator mode 
}

function updateCalculatorModeButton() {
    var calculatorModeBtn = document.getElementById("calculatorModeBtn");
    if (calculatorMode) {
        calculatorModeBtn.classList.add("calculator-mode-enabled");
        calculatorModeBtn.classList.remove("calculator-mode-disabled");
    } 
    else {
        calculatorModeBtn.classList.remove("calculator-mode-enabled");
        calculatorModeBtn.classList.add("calculator-mode-disabled");
    }
}

function updateOperatorButtons() {
    var operatorButtons = document.getElementsByClassName("operator");
    for (var i = 0; i < operatorButtons.length; i++) {
        if (calculatorMode) {
            operatorButtons[i].removeAttribute("disabled");
        } 
        else {
            operatorButtons[i].setAttribute("disabled", "disabled");
        }
    }
}

function addOperation(operation) {
  if(calculatorMode) {
      if (currentOperation === null) {
          currentOperation = operation;
          currentValue = parseFloat(document.getElementById("position").innerText);
          tempNumber = "";
      }
  }
}

function addDecimal() {
    // Ensure tempNumber doesn't already contain a decimal point
    if (!tempNumber.includes('.')) {
        tempNumber += '.';
    }
    updateDisplay();
}

function addNumber(number) {
    tempNumber += number.toString();
    updateDisplay();
}

function calculate() {
    if (currentOperation !== null && tempNumber !== "") {
        switch (currentOperation) {
            case '+':
                currentValue += parseFloat(tempNumber);
                break;
            case '*':
                currentValue *= parseFloat(tempNumber);
                break;
            case '-':
                currentValue -= parseFloat(tempNumber);
                break;
            case '/':
                var divisor = parseFloat(tempNumber);
                if (divisor !== 0) {
                  currentValue /= divisor;
                } 
                else {
                  alert("Nuclear threat detected");
                }
                break;
        }
        currentOperation = null;
        tempNumber = "";
        updateDisplay();
    }
}

function updateDisplay() {
    var positionElement = document.getElementById("calctemp");
    positionElement.innerText = currentValue.toString();
}

function zeroAllAxis() {
    fetch("/zero-all-axis")
    .then(response => {
        if (!response.ok) {
            throw new Error('Network response was not ok');
        }
        return response.text();
    })
    .then(data => {
        console.log(data); // Log the server response for debugging.
        alert(data); // Alert the user or update the status on the page.
    })
    .catch(error => {
        console.error('There was a problem with the fetch operation:', error);
        alert("Failed to zero all axes: " + error.message); // Provide error feedback.
    });
}


function toggleMode() {
  fetch("/toggle-mode")
  .then(response => response.text())
  .then(data => {
    // Now using 'modeIndicator' as the ID for the mode display element
    document.getElementById("modeIndicator").innerText = data;
    // updatePositions(); // Update positions if needed, otherwise you can remove this line         // off for atm
  })
  .catch(console.error);
}

function toggleMeasureMode() {
  fetch("/toggle-measure-mode")
  .then(response => response.text())
  .then(data => {
    // Now using 'modeIndicator' as the ID for the mode display element
    document.getElementById("modeMeasureIndicator").innerText = data;
    // updatePosition(); // Update positions if needed, otherwise you can remove this line
  })
  .catch(console.error);
}

 
function updatePosition() {
  fetch("/get-positions")
  .then(response => response.json())
  .then(data => {
    document.getElementById("position").innerText = data.positionX;
    document.getElementById("position2").innerText = data.positionY;
    document.getElementById("position3").innerText = data.positionZ;
  })
  .catch(console.error);
}

// Call updatePosition at an interval
setInterval(updatePosition, 100);  // Update every second

  
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

function updatePlaneDisplay() {
    fetch("/get-current-plane")
    .then(response => response.text())
    .then(data => {
        console.log("Updated Plane Index: ", data); // Log for debugging
        document.getElementById("currentPlane").innerText = "Plane: " + data;
    })
    .catch(error => console.error("Failed to update plane display:", error));
}


 
function updatePositionsAndPlane() {
    updatePosition();
    updatePlaneDisplay();
}

 

  setInterval(updatePositionsAndPlane, 1000); // Adjust interval to 1000 ms
  setInterval(updatePosition, 50);   // Call updatePositions() every 1000ms (1 second) but right now it is 50ms so stupid fast 
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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/html", index_html); });


  // server.on("/poss", HTTP_GET, [](AsyncWebServerRequest *request)
  //           {
  //             String position1;
  //             String position2;
  //             String position3;

  //             if (isABSMode)
  //             {
  //               position1 = String(encoderValueABS[0]);
  //               position2 = String(encoderValueABS[1]);
  //               position3 = String(encoderValueABS[2]);
  //             }
  //             else
  //             {
  //               position1 = String(encoderValueINC[0]);
  //               position2 = String(encoderValueINC[1]);
  //               position3 = String(encoderValueINC[2]);
  //             }

  //             // This might be the issue since we are converting to measure mode when I think this is jsut in charge of switching from abs to inc
  //             StaticJsonDocument<200> jsonDoc;
  //             // jsonDoc["position1"] = formatPosition(position1.toFloat(), isABSMode);
  //             // jsonDoc["position2"] = formatPosition(position2.toFloat(), isABSMode);
  //             // jsonDoc["position3"] = formatPosition(position3.toFloat(), isABSMode);

  //             jsonDoc["position1"] = position1;
  //             jsonDoc["position2"] = position2;
  //             jsonDoc["position3"] = position3;

  //             String jsonString;
  //             serializeJson(jsonDoc, jsonString);

  //             request->send(200, "application/json", jsonString); // Send JSON data
  //           });

  server.on("/get-positions", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              char formattedX[20], formattedY[20], formattedZ[20];

              // Assuming encoder1, encoder2, encoder3 are your encoder instances
              String positionX = formatPosition(isABSMode ? encoder1.position - X_last_ABS : encoder1.position - X_last_INC, isInchMode);
              String positionY = formatPosition(isABSMode ? encoder2.position - Y_last_ABS : encoder2.position - Y_last_INC, isInchMode);
              String positionZ = formatPosition(isABSMode ? encoder3.position - Z_last_ABS : encoder3.position - Z_last_INC, isInchMode);

              // Create a JSON object to send the formatted positions
              StaticJsonDocument<200> jsonDoc;
              jsonDoc["positionX"] = positionX;
              jsonDoc["positionY"] = positionY;
              jsonDoc["positionZ"] = positionZ;

              String jsonString;
              serializeJson(jsonDoc, jsonString);

              request->send(200, "application/json", jsonString); // Send JSON data
            });

  server.on("/toggle-measure-mode", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    toggleMeasurementMode();
    request->send(200, "text/plain", isInchMode ? "INCH" : "MM"); });

  server.on("/reset/x", HTTP_GET, [](AsyncWebServerRequest *request)
            {         
            resetEncoderValue(0); // Reset encoder for X-axis

    request->send(200, "text/plain", "X position reset"); });

  server.on("/reset/y", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    resetEncoderValue(1); // Reset encoder for Y-axis
    request->send(200, "text/plain", "Y position reset"); });

  server.on("/reset/z", HTTP_GET, [](AsyncWebServerRequest *request)
            {
    resetEncoderValue(2); // Reset encoder for Z-axis
    request->send(200, "text/plain", "Z position reset"); });

  server.on("/toggle-mode", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              toggleMode();
              request->send(200, "text/plain", isABSMode ? "ABS" : "INC"); // Send the new mode back to the client
            });
    
  server.on("/zero-all-axis", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                resetEncoderValue(0);
                resetEncoderValue(1);
                resetEncoderValue(2);
                request->send(200, "text/plain", "All positions reset"); 
              });

server.on("/get-current-plane", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(currentPlaneIndex + 1));  // +1 to make it human-readable (1-based index)
});




  // server.on("/reset-encoder", HTTP_GET, [](AsyncWebServerRequest *request)
  //           {
  // if (request->hasParam("encoder")) {
  //   auto* param = request->getParam("encoder");
  //   int encoderIndex = param->value().toInt();
  //   resetEncoderValue(encoderIndex);
  //   request->send(200, "text/plain", "Reset done");
  // } else {
  //   request->send(400, "text/plain", "Missing encoder parameter");
  // } });

  // Axis Selector buttons

  server.begin();

  // Monitor pin setup //
  attachInterrupt(digitalPinToInterrupt(encoder1.pinA), handleEncoder1Interrupt, CHANGE); // I need to remove the lambda and include the w/ name of interrupt to ensure we are using the correct one

  attachInterrupt(digitalPinToInterrupt(encoder1.pinB), handleEncoder1Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder2.pinA), handleEncoder2Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder2.pinB), handleEncoder2Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder3.pinA), handleEncoder3Interrupt, CHANGE);

  attachInterrupt(digitalPinToInterrupt(encoder3.pinB), handleEncoder3Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder4.pinA), handleEncoder4Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder4.pinB), handleEncoder4Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder5.pinA), handleEncoder5Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder5.pinB), handleEncoder5Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder6.pinA), handleEncoder6Interrupt, CHANGE);

  // attachInterrupt(digitalPinToInterrupt(encoder6.pinB), handleEncoder6Interrupt, CHANGE);

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
    char buffer[128]; // Make sure the buffer is large enough to hold the string
    int xOffset; // Horizontal offset to right-align text

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
        // Display the X and Y axes for the two-axis mode
        displayAxisValues(0, 0); // X-axis
        displayAxisValues(1, 16); // Y-axis
        LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // Return option
        break;

    case THREE_AXIS:
        // Display the X, Y, and Z axes for the three-axis mode including the plane index
        snprintf(buffer, sizeof(buffer), "Plane %d - %s Mode", currentPlaneIndex + 1, isABSMode ? "ABS" : "INC");
        LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK); // Display the plane and mode at the top
        displayAxisValues(0, 16); // X-axis
        displayAxisValues(1, 32); // Y-axis
        displayAxisValues(2, 48); // Z-axis
        LCDTextDraw(0, 64, "> return ", 1, WHITE, BLACK); // Return option
        break;
    }
}




// void updateDisplayContent()
// {
//   char buffer[11];
//   int xOffset; // Horizontal offset to right-align text

//   switch (currentMenuState)
//   {
//   case MAIN_MENU:
//     LCDTextDraw(7, 0, " COMP491 ESP32 DRO ", 1, WHITE, BLACK);
//     for (int i = 0; i < 2; i++)
//     {
//       sprintf(buffer, "%s %s", (i == menuItemIndex) ? ">" : " ", MenuDroItems[i]);
//       LCDTextDraw(0, 16 * (i + 1), buffer, 1, WHITE, BLACK);
//     }
//     break;
//   case TWO_AXIS:
//     LCDRectFill(0, 0, 50, 10, BLACK); // Fill a rectangle area with BLACK to clear previous number
//                                       // Format and right-align "X" axis label and value
//     snprintf(buffer, sizeof(buffer), "X: %s", formatPosition(encoder1.position, isInchMode).c_str());
//     xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
//     LCDTextDraw(xOffset, 0, buffer, 1, WHITE, BLACK);

//     // Format and right-align "Y" axis label and value
//     snprintf(buffer, sizeof(buffer), "Y: %s", formatPosition(encoder2.position, isInchMode).c_str());
//     xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
//     LCDTextDraw(xOffset, 16, buffer, 1, WHITE, BLACK);
//     break;

//     LCDRectFill(0, 50, 50, 10, BLACK);
//     LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
//     break;
//   case THREE_AXIS:
//     // Format and right-align "X" axis label and value
//     snprintf(buffer, sizeof(buffer), "X: %s", formatPosition(encoder1.position, isInchMode).c_str());
//     xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
//     LCDTextDraw(xOffset, 0, buffer, 1, WHITE, BLACK);

//     // Format and right-align "Y" axis label and value
//     snprintf(buffer, sizeof(buffer), "Y: %s", formatPosition(encoder2.position, isInchMode).c_str());
//     xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
//     LCDTextDraw(xOffset, 16, buffer, 1, WHITE, BLACK);

//     // Format and right-align "Z" axis label and value
//     snprintf(buffer, sizeof(buffer), "Z: %s", formatPosition(encoder3.position, isInchMode).c_str());
//     xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
//     LCDTextDraw(xOffset, 32, buffer, 1, WHITE, BLACK);
//     break;
//     // LCDRectFill(0, 0, 50, 10, BLACK);
//     // sprintf(buffer, "X: %d", encoder4.position);
//     // LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);

//     // LCDRectFill(0, 16, 50, 10, BLACK);
//     // sprintf(buffer, "Y: %d", encoder5.position);
//     // LCDTextDraw(0, 16, buffer, 1, WHITE, BLACK);

//     // LCDRectFill(0, 32, 50, 10, BLACK);
//     // sprintf(buffer, "Z: %d", encoder6.position);
//     // LCDTextDraw(0, 32, buffer, 1, WHITE, BLACK);

//     LCDRectFill(0, 50, 50, 10, BLACK);
//     LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // menu option to return
//     break;
//   }
// }

// THIS UPDATE DISPLAY HANDLES MODE SWITCHING AND MEASURE MODE // 

// void updateDisplayContent() {
//     char buffer[32];  // Increased buffer size for larger strings
//     int xOffset;      // Horizontal offset to right-align text

//     switch (currentMenuState) {
//         case MAIN_MENU:
//             LCDTextDraw(7, 0, " COMP491 ESP32 DRO ", 1, WHITE, BLACK);
//             for (int i = 0; i < 2; i++) {
//                 sprintf(buffer, "%s %s", (i == menuItemIndex) ? ">" : " ", MenuDroItems[i]);
//                 LCDTextDraw(0, 16 * (i + 1), buffer, 1, WHITE, BLACK);
//             }
//             break;

//         case TWO_AXIS:
//             // Clear the areas for fresh update
//             LCDRectFill(0, 0, SCREEN_WIDTH, 32, BLACK); // Clear area for X and Y axis values

//             // Display X Axis
//             snprintf(buffer, sizeof(buffer), "X: %s", formatPosition(isABSMode ? encoder1.position - X_last_ABS : encoder1.position - X_last_INC, isInchMode).c_str());
//             xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH);
//             LCDTextDraw(xOffset, 0, buffer, 1, WHITE, BLACK);

//             // Display Y Axis
//             snprintf(buffer, sizeof(buffer), "Y: %s", formatPosition(isABSMode ? encoder2.position - Y_last_ABS : encoder2.position - Y_last_INC, isInchMode).c_str());
//             xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH);
//             LCDTextDraw(xOffset, 16, buffer, 1, WHITE, BLACK);

//             LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK);
//             break;

//         case THREE_AXIS:
//             // Clear the entire display area or just the areas being updated to manage flickering
//             LCDRectFill(0, 0, SCREEN_WIDTH, 48, BLACK); // Clear area for X, Y, Z axis values

//             // Display X Axis
//             snprintf(buffer, sizeof(buffer), "X: %s", formatPosition(isABSMode ? encoder1.position - X_last_ABS : encoder1.position - X_last_INC, isInchMode).c_str());
//             xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH);
//             LCDTextDraw(xOffset, 0, buffer, 1, WHITE, BLACK);

//             // Display Y Axis
//             snprintf(buffer, sizeof(buffer), "Y: %s", formatPosition(isABSMode ? encoder2.position - Y_last_ABS : encoder2.position - Y_last_INC, isInchMode).c_str());
//             xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH);
//             LCDTextDraw(xOffset, 16, buffer, 1, WHITE, BLACK);

//             // Display Z Axis
//             snprintf(buffer, sizeof(buffer), "Z: %s", formatPosition(isABSMode ? encoder3.position - Z_last_ABS : encoder3.position - Z_last_INC, isInchMode).c_str());
//             xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH);
//             LCDTextDraw(xOffset, 32, buffer, 1, WHITE, BLACK);

//             LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK);
//             break;
//     }
// }


void TaskUpdateDisplay(void *pvParameters)
{
  for (;;)
  {
    // // Shows us the angle of current encoder pos for encoders [1,2,3]
    // long currentPulses1 = encoder1.position; // This should be the net count considering direction
    // float angleTurned1 = pulsesToDegrees(currentPulses1);

    // long currentPulses2 = encoder2.position; // This should be the net count considering direction
    // float angleTurned2 = pulsesToDegrees(currentPulses2);

    // long currentPulses3 = encoder3.position; // This should be the net count considering direction
    // float angleTurned3 = pulsesToDegrees(currentPulses3);

    // Serial.print("Encoder1 Angle Turned: ");
    // Serial.println(angleTurned1);

    // Serial.print("Encoder2 Angle Turned: ");
    // Serial.println(angleTurned2);

    // Serial.print("Encoder3 Angle Turned: ");
    // Serial.println(angleTurned3);

    // Serial.println("----------------------");

    // // Shows us the INCH of current encoder pos for encoders [1,2,3]
    // float distanceMovedInches1 = pulsesToDistanceInches(currentPulses1);
    // float distanceMovedInches2 = pulsesToDistanceInches(currentPulses2);
    // float distanceMovedInches3 = pulsesToDistanceInches(currentPulses3);

    // Serial.print("Encoder1 Distance Moved: ");
    // Serial.print(distanceMovedInches1);
    // Serial.println(" inches");

    // Serial.print("Encoder2 Distance Moved: ");
    // Serial.print(distanceMovedInches2);
    // Serial.println(" inches");

    // Serial.print("Encoder3 Distance Moved: ");
    // Serial.print(distanceMovedInches3);
    // Serial.println(" inches");

    handleMenuNavigation();
    updateDisplayContent();
    // vTaskDelay(pdMS_TO_TICKS(100));
  }
}











/* Add manner we can switch from different Coordinate Planes on the (HTML or on OLED screen)
   Based on the index of the current plane, store multiple points to start drawing shapes 
   Display the number of the current Coordinate Plane in use
   Create display to show the coordinates being displayed 
   Only store ABS values in its own Coordinate plane */
   
   