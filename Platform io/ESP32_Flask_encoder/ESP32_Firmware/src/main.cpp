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


#define SCREEN_WIDTH 128 // OLED display width
#define CHAR_WIDTH 6     // Width of each character in pixels



// Define LED colors as global constants
const int LEDColorDisconnected[3] = {0, 0, 0};
const int LEDColorPurple[3] = {128, 0, 128};
const int LEDColorTurquoise[3] = {83, 195, 189};
const int LEDColorPink[3] = {255, 292, 203};





// Forward declarations
void TaskNetwork(void *pvParameters);
void TaskUpdateDisplay(void *pvParameters);



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

bool isABSMode = true; // Start in ABS mode
int currentPlaneIndex = 0; // Keep track of the current plane index

// Initializing encoders attributes and setting their start (refer to encoder struct to see all parameters)
Encoder encoder1 = {PIN_A1, PIN_B1, 0, 0, 0, 0, 0, {0}, 1};
Encoder encoder2 = {PIN_A2, PIN_B2, 0, 0, 0, 0, 0, {0}, 1};
Encoder encoder3 = {PIN_A3, PIN_B3, 0, 0, 0, 0, 0, {0}, 1};

// Encoder encoder4 = {PIN_A4, PIN_B4, 0, 0, 0, 0, {0}, 1};
// Encoder encoder5 = {PIN_A5, PIN_B5, 0, 0, 0, 0, {0}, 1};
// Encoder encoder6 = {PIN_A6, PIN_B6, 0, 0, 0, 0, {0}, 1};

// void IRAM_ATTR handleEncoder1Interrupt() { updateEncoder(&encoder1); }
// void IRAM_ATTR handleEncoder2Interrupt() { updateEncoder(&encoder2); }
// void IRAM_ATTR handleEncoder3Interrupt() { updateEncoder(&encoder3); }
// void IRAM_ATTR handleEncoder4Interrupt() { updateEncoder(&encoder4); }
// void IRAM_ATTR handleEncoder5Interrupt() { updateEncoder(&encoder5); }
// void IRAM_ATTR handleEncoder6Interrupt() { updateEncoder(&encoder6); }



struct Point
{
    int x, y, z; // Include z if you plan to extend to 3D shapes

    Point(int px, int py, int pz = 0) : x(px), y(py), z(pz) {} // Constructor for initialization
};

struct CoordinatePlane
{
    std::vector<Point> shapePoints; // Store points for each shape in the plane
    int encoderValueABS[3];         // Store ABS values for X, Y, Z
    int encoderValueINC[3];         // Store INC values for X, Y, Z
    int last_ABS[3];                // Last ABS position for X, Y, Z
    int last_INC[3];                // Last INC position for X, Y, Z
};

CoordinatePlane planes[12];

void selectPlane(int index)
{
    if (index >= 0 && index < 12)
    {
        currentPlaneIndex = index;
        refreshAndDrawPoints(); // Refresh display after switching planes
    }
    // Update display or other state changes needed when switching planes (needs to be implemented)
    // updateDisplayContent();
}

void nextPlane()
{
    selectPlane((currentPlaneIndex + 1) % 12);
    Serial.print("Next Plane: ");
    Serial.println(currentPlaneIndex + 1);
}

void previousPlane()
{
    selectPlane((currentPlaneIndex + 11) % 12);
    Serial.print("Previous Plane: ");
    Serial.println(currentPlaneIndex + 1);
}
void addCurrentPositionToPoint()
{
    int currentX = planes[currentPlaneIndex].encoderValueABS[0]; // or encoderValueINC based on mode
    int currentY = planes[currentPlaneIndex].encoderValueABS[1]; // or encoderValueINC based on mode
    int currentZ = planes[currentPlaneIndex].encoderValueABS[2]; // or encoderValueINC based on mode

    // Adds the current position as a new point to the current plane
    planes[currentPlaneIndex].shapePoints.emplace_back(currentX, currentY, currentZ);
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
void resetEncoderValue(int encoderIndex) {
    if (encoderIndex < 0 || encoderIndex >= 3) {
        Serial.println("Error: Encoder index out of range");
        return;
    }

    // Get references to the current plane for easier access
    auto &currentPlane = planes[currentPlaneIndex];
    auto &lastABS = currentPlane.last_ABS;
    auto &lastINC = currentPlane.last_INC;
    auto &encoderValueABS = currentPlane.encoderValueABS;
    auto &encoderValueINC = currentPlane.encoderValueINC;

    Encoder &encoder = (encoderIndex == 0) ? encoder1 : (encoderIndex == 1) ? encoder2 : encoder3;

    if (isABSMode) {
        encoderValueABS[encoderIndex] = 0; // Zero out the ABS value
        lastABS[encoderIndex] = encoder.position; // Store last ABS position
    } else {
        encoderValueINC[encoderIndex] = 0; // Zero out the INC value
        lastINC[encoderIndex] = encoder.position; // Store last INC position
    }
}



// void resetEncoderValue(int encoderIndex)
// {
//     if (encoderIndex < 0 || encoderIndex >= 3)
//     {
//         Serial.println("Error: Encoder index out of range");
//         return; // Add error handling or user feedback
//     }
//     // Placeholder for resetting encoder value.
//     // if we zero out a value we need to store the last position before zeroing out to calculate the difference of (encoder.position - lastValVisited)
//     if (isABSMode)
//     {
//         // reset value back to 0
//         planes[currentPlaneIndex].encoderValueABS[encoderIndex] = 0;

//         // store last coordinate (value) listed
//         if (encoderIndex == 0)
//         {

//             planes[currentPlaneIndex].last_ABS[0] = encoder1.position;
//         }
//         else if (encoderIndex == 1)
//         {
//             planes[currentPlaneIndex].last_ABS[1] = encoder2.position;
//         }
//         else if (encoderIndex == 2)
//         {
//             planes[currentPlaneIndex].last_ABS[2] = encoder3.position;
//         }
//     }
//     else
//     {
//         // reset value back to 0
//         planes[currentPlaneIndex].encoderValueINC[encoderIndex] = 0;

//         // store last coordinate (value) listed
//         if (encoderIndex == 0)
//         {
//             planes[currentPlaneIndex].last_INC[0] = encoder1.position;
//         }
//         else if (encoderIndex == 1)
//         {
//             planes[currentPlaneIndex].last_INC[1] = encoder2.position;
//         }
//         else if (encoderIndex == 2)
//         {
//             planes[currentPlaneIndex].last_INC[2] = encoder3.position;
//         }
//     }
//     // Consider adding logic to update the display or take other actions.
// }

// void resetEncoderValue(int encoderIndex) {
//     if (encoderIndex < 0 || encoderIndex >= 3) {
//         Serial.println("Error: Encoder index out of range");
//         return;
//     }

//     if (isABSMode) {
//         // Reset the ABS value to zero and store the last position
//         planes[currentPlaneIndex].encoderValueABS[encoderIndex] = 0;
//         switch (encoderIndex) {
//             case 0:
//                 X_last_ABS[currentPlaneIndex] = encoder1.position;
//                 break;
//             case 1:
//                 Y_last_ABS[currentPlaneIndex] = encoder2.position;
//                 break;
//             case 2:
//                 Z_last_ABS[currentPlaneIndex] = encoder3.position;
//                 break;
//         }
//     } else {
//         // Reset the INC value to zero and store the last position
//         planes[currentPlaneIndex].encoderValueINC[encoderIndex] = 0;
//         switch (encoderIndex) {
//             case 0:
//                 X_last_INC[currentPlaneIndex] = encoder1.position;
//                 break;
//             case 1:
//                 Y_last_INC[currentPlaneIndex] = encoder2.position;
//                 break;
//             case 2:
//                 Z_last_INC[currentPlaneIndex] = encoder3.position;
//                 break;
//         }
//     }
// }

// Helper function to format and display axis values based on current settings
void displayAxisValues(int axis, int yPosition)
{
    char buffer[40];
    int xOffset;
    int position = isABSMode ? planes[currentPlaneIndex].encoderValueABS[axis] : planes[currentPlaneIndex].encoderValueINC[axis];
    snprintf(buffer, sizeof(buffer), "%c: %s", 'X' + axis, formatPosition(position, isInchMode).c_str());
    xOffset = SCREEN_WIDTH - (strlen(buffer) * CHAR_WIDTH); // Calculate x offset for right alignment
    LCDTextDraw(xOffset, yPosition, buffer, 1, WHITE, BLACK);
}

void handleMenuNavigation()
{
    // Navigate menu options in the main menu
    if (currentMenuState == MAIN_MENU)
    {
        if (ButtonUpPressed() && !ButtonStatesPrev[2])
        {
            menuItemIndex = max(0, menuItemIndex - 1);
        }
        else if (ButtonDownPressed() && !ButtonStatesPrev[3])
        {
            menuItemIndex = min(1, menuItemIndex + 1); // Only two options
        }
        else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
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
    }
    // Handle plane navigation in the TWO_AXIS state
    else if (currentMenuState == TWO_AXIS)
    {
        if (ButtonLeftPressed() && !ButtonStatesPrev[0])
        {
            previousPlane();
            // updateDisplayContent();  // Show updated plane information
        }
        else if (ButtonRightPressed() && !ButtonStatesPrev[4])
        {
            nextPlane();
            // updateDisplayContent();  // Show updated plane information
        }
        else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
        {
            currentMenuState = MAIN_MENU; // Return to main menu on center button
            LCDScreenClear();
        }
    }
    // Handle plane navigation in the THREE_AXIS state
    else if (currentMenuState == THREE_AXIS)
    {
        if (ButtonLeftPressed() && !ButtonStatesPrev[0])
        {
            previousPlane();
            // updateDisplayContent();  // Show updated plane information
        }
        else if (ButtonRightPressed() && !ButtonStatesPrev[4])
        {
            nextPlane();
            // updateDisplayContent();  // Show updated plane information
        }
        else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
        {
            currentMenuState = MAIN_MENU; // Return to main menu on center button
            LCDScreenClear();
        }
    }
    // Update the stored state of buttons after handling logic
    updateButtonStates();
}

void IRAM_ATTR handleEncoder1Interrupt()
{
    handleEncoderInterrupt(&encoder1); // Assume encoder1 is an instance of Encoder
                                       // for all axis on ABS Mode
    // handleEncoderInterrupt(&encoder1); // Update encoder state

    planes[currentPlaneIndex].encoderValueABS[0] = encoder1.position - planes[currentPlaneIndex].last_ABS[0];
    planes[currentPlaneIndex].encoderValueINC[0] = encoder1.position - planes[currentPlaneIndex].last_INC[0];

    // // Optionally add point on certain condition but we might not needs this at all
    //   if (some_condition_met) {
    //       addPointToPlane(currentPlaneIndex, encoder1.position, encoder2.position);
    //   }
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder2Interrupt()
{
    handleEncoderInterrupt(&encoder2); // Assume encoder1 is an instance of Encoder
    planes[currentPlaneIndex].encoderValueABS[1] = encoder2.position - planes[currentPlaneIndex].last_ABS[1];
    planes[currentPlaneIndex].encoderValueINC[1] = encoder2.position - planes[currentPlaneIndex].last_INC[1];
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder3Interrupt()
{
    handleEncoderInterrupt(&encoder3); // Update encoder state
    planes[currentPlaneIndex].encoderValueABS[2] = encoder3.position - planes[currentPlaneIndex].last_ABS[2];
    planes[currentPlaneIndex].encoderValueINC[2] = encoder3.position - planes[currentPlaneIndex].last_INC[2];
}

// void IRAM_ATTR handleEncoder1Interrupt() {
//     handleEncoderInterrupt(&encoder1); // Handle the encoder update
//     planes[currentPlaneIndex].encoderValueABS[0] = encoder1.position - X_last_ABS[currentPlaneIndex];
//     planes[currentPlaneIndex].encoderValueINC[0] = encoder1.position - X_last_INC[currentPlaneIndex];
// }

// void IRAM_ATTR handleEncoder2Interrupt() {
//     handleEncoderInterrupt(&encoder2);
//     planes[currentPlaneIndex].encoderValueABS[1] = encoder2.position - Y_last_ABS[currentPlaneIndex];
//     planes[currentPlaneIndex].encoderValueINC[1] = encoder2.position - Y_last_INC[currentPlaneIndex];
// }

// void IRAM_ATTR handleEncoder3Interrupt() {
//     handleEncoderInterrupt(&encoder3);
//     planes[currentPlaneIndex].encoderValueABS[2] = encoder3.position - Z_last_ABS[currentPlaneIndex];
//     planes[currentPlaneIndex].encoderValueINC[2] = encoder3.position - Z_last_INC[currentPlaneIndex];
// }

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

void updateDisplayWithPoints()
{
    LCDScreenClear(); // Clear the screen for fresh update
    drawGrid();       // Draw the grid

    // Draw each point in the current plane
    for (const auto &point : planes[currentPlaneIndex].shapePoints)
    {
        drawPoint(point.x, point.y);
    }
}

void refreshAndDrawPoints()
{
    // LCDScreenClear(); // temp removed

    // Iterate over all points in the current plane and draw them
    for (const auto &point : planes[currentPlaneIndex].shapePoints)
    {
        drawPointOnOLED(point.x, point.y);
    }

    // LCD.display();  // Refresh the display to show all points
}

void addPointToCurrentPlane(int x, int y, int z = 0)
{
    planes[currentPlaneIndex].shapePoints.emplace_back(x, y, z);
    refreshAndDrawPoints(); // Refresh display after adding point
}

// Functions have not been implemented //
void removeLastPointFromCurrentPlane(int planeIndex)
{
    if (planeIndex >= 0 && planeIndex < 12 && !planes[planeIndex].shapePoints.empty())
    {
        planes[planeIndex].shapePoints.pop_back();
        // updateDisplayContent();
    }
}

void displayCurrentPoints(int planeIndex)
{
    if (planeIndex >= 0 && planeIndex < 12)
    {
        auto &points = planes[planeIndex].shapePoints;
        for (size_t i = 0; i < points.size(); ++i)
        {
            char buffer[50];
            snprintf(buffer, sizeof(buffer), "Point %zu: (%d, %d, %d)", i + 1, points[i].x, points[i].y, points[i].z);
            LCDTextDraw(0, i * 16, buffer, 1, WHITE, BLACK); // Adjust positioning as needed
        }
    }
}

void clearPointsInPlane(int planeIndex)
{
    if (planeIndex >= 0 && planeIndex < 12)
    {
        planes[planeIndex].shapePoints.clear();
        // updateDisplayContent();
    }
}


const char *h_ssid = "491-DRO-Boyyz";
const char *h_password = "123456789";

const int ledPin = 12;  // The GPIO pin connected to your LED strip
const int numLeds = 12; // Number of LEDs in your strip

CRGB leds[numLeds];
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1.0" />
    <title>DRO Interface</title>
    <style>
        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background-color: #e7e7e7;
            color: #333;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
        }

        .dro-container {
            background-color: #d8d8d8;
            padding: 20px;
            border-radius: 8px;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
            margin-bottom: 20px;
        }

        .readout {
            background-color: #000;
            color: #0f0;
            padding: 20px;
            font-size: 1.5em;
            font-family: 'Courier New', Courier, monospace;
            border: 1px solid #333;
            margin-bottom: 10px;
        }



        /* Mode Display */
        #container {
            position: absolute;
            top: 50px;
            left: 50px;
            width: 150px;
            height: 285px;
        }

        /* DRO readout */
        #container1 {
            position: absolute;
            top: 50px;
            left: 250px;
            width: 450px;
        }

        /* Function controls */
        #container2 {
            position: absolute;
            top: 390px;
            left: 393px;
        }

        /* Save coordinate container */
        #container3 {
            position: absolute;
            top: 390px;
            left: 50px;
            width: 293px;
            height: 220px;
        }

        /* Coordinate list*/
        #container12 {
            position: absolute;
            top: 660px;
            left: 50px;
            width: 650px;
            height: 123px;

        }

        /* Zero ut / Selector Buttons */
        #container4 {
            position: absolute;
            top: 50px;
            left: 750px;
            bottom: 490px;
            max-width: 400px;
            height: 130px;
        }

        /* Function buttons */
        #container5 {
            position: absolute;
            top: 810px;
            left: 50px;
        }

        /* Shape helper functions buttons */
        #container6 {
            position: absolute;
            top: 490px;
            left: 393px;
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 5px;
            width: 308px;
            height: 120px;
        }

        /* Arrow Buttons */
        #container7 {
            position: absolute;
            left: 750px;
            top: 645px;
            height: 140px;
            display: flex;
            flex-direction: column;
            align-items: center;
            width: 160px;
            /* Adjust if necessary */
        }

        .arrow-row {
            display: flex;
            justify-content: center;
            align-items: center;
        }

        /* Style adjustments for the first row with the Up arrow */
        .arrow-row.up {
            margin-bottom: 5px;
            /* Space between the up arrow and the middle row */
        }

        .arrow-row.middle {
            width: 100%;
            /* Ensures the middle row takes full width of container */
            justify-content: space-around;
            /* Evenly spaces items in this row */
        }

        .arrow-row button {
            width: 50px;
            /* Width of each button */
            height: 56px;
            /* Height of each button */
            border-radius: 4px;
            background-color: #f5f5f5;
            /* Background color */
            border: 1px solid #ccc;
            /* Border color */
        }

        .arrow-row button:hover {
            background-color: #e0e0e0;
            /* Darker background on hover */
        }

        /* Ensuring that the middle row buttons (left and right) are visually aligned and square */
        .arrow-row.middle button {
            width: 56px;
            /* Making them square */
        }

        /* Adding icons to the buttons (if you use something like Font Awesome) */
        .arrow-row button.up-arrow:before {
            content: '↑';
            /* Up arrow icon */
        }

        .arrow-row button.down-arrow:before {
            content: '↓';
            /* Down arrow icon */
        }

        .arrow-row button.left-arrow:before {
            content: '←';
            /* Left arrow icon */
        }

        .arrow-row button.right-arrow:before {
            content: '→';
            /* Right arrow icon */
        }



        /* Calculator Display*/
        #container11 {
            position: absolute;
            top: 50px;
            left: 965px;
            width: 370px;
        }

        /* Calculator */
        #container8 {
            position: absolute;
            top: 160px;
            left: 965px;
        }

        /* ABS/INC Toggle Buttons, Zero all out etc */
        #container9 {
            position: absolute;
            top: 430px;
            left: 750px;
            width: 160px;

        }

        /* Pulse Factor Adjustment */
        #container10 {
            position: absolute;
            top: 235px;
            left: 750px;
            width: 160px;
            height: 140px;

        }

        /* Coordinate plane display */
        #container13 {
            position: absolute;
            top: 605px;
            left: 970px;
            width: 365px;
            height: 185px;
        }







        .calculator-grid {
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
            background: #f5f5f5;
            /* Lighter background */
            border: 1px solid #bbb;
            /* Softer border color */
            border-radius: 5px;
            text-align: center;
            cursor: pointer;
            transition: background-color 0.3s;
            /* Smooth transition for hover effect */
        }

        button:hover {
            background: #e0e0e0;
            /* Subtle hover effect */
        }

        /* .wide {
            grid-column: span 3;
        } */

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

        /* Media Queries */

        /* Smaller screens (tablets) */
        @media (max-width: 768px) {
            body {
                flex-direction: column;
            }

            .dro-container,
            #container,
            #container1,
            #container2,
            #container3,
            #container4,
            #container5,
            #container6,
            #container7,
            #container8,
            #container9,
            #container10,
            #container11 {
                position: relative;
                width: 90%;
                max-width: none;
                margin: 10px auto;
            }

            .readout {
                font-size: 1.2em;
                /* Smaller font for smaller screens */
            }
        }

        /* Very small screens (phones) */
        @media (max-width: 480px) {
            .readout {
                padding: 10px;
                font-size: 1em;
                /* Even smaller font for phone screens */
            }

            button {
                padding: 10px;
                font-size: 0.8em;
                /* Smaller buttons for easier handling */
            }
        }
    </style>
</head>

<body>


    <!-- Container 1: Readout displays (X, Y, Z) -->

    <div class="dro-container" id="container13">
        <canvas id="coordinateCanvas" width="400" height="400"></canvas>
    </div>


    <div class="dro-container" id="container">
        <div class="readout" id="currentPlane">1</div>
        <div class="readout" id="modeIndicator">ABS</div>

        <div class="readout modeDisplay" id="mode-readout">
            <span id="modeMeasureIndicator">INCH</span>
        </div>

    </div>

    <div class="dro-container" id="container1">

        <div class="readout" id="x-readout">
            X: <span id="position"></span>
        </div>
        <div class="readout" id="y-readout">
            Y: <span id="position2"></span>
        </div>
        <div class="readout" id="z-readout">
            Z: <span id="position3"></span>
        </div>
        <button onclick="saveCurrentPosition()">
            Save Current Position
        </button>
    </div>

    <!-- Container 2: Function keys -->
    <div class="dro-container" id="container2">
        <button>F1</button>
        <button>F2</button>
        <button>F3</button>
        <button>F4</button>
        <button>F5</button>
        <button>F6</button>
    </div>


    <div class="dro-container" id="container3">
        <div class="dro-container">
            <input type="number" id="xInput" placeholder="X-coordinate" />
            <input type="number" id="yInput" placeholder="Y-coordinate" />
            <input type="number" id="zInput" placeholder="Z-coordinate" />
            <div id="lastPoint">Last Point: None</div>
        </div>
        <button onclick="savePoint()">Save Point</button>
        <button onclick="getAllPoints()">Show All Points</button>
    </div>

    <div class="dro-container" id="container12">

        <ul id="pointsList"></ul>
    </div>



    <div class="dro-container" id="container4">
        <div class="zeroSelect-grid">
            <button onclick="resetPosition('x')">Xo</button>
            <button onclick="resetPosition('select_x')">Select X</button>
            <button onclick="resetPosition('y')">Yo</button>
            <button onclick="resetPosition('select_y')">Select Y</button>
            <button onclick="resetPosition('z')">Zo</button>
            <button onclick="resetPosition('select_z')">Select Z</button>

        </div>
    </div>

    <div class="dro-container" id="container10">
        <form id="factorForm">
            <!-- <label for="factor_mm">Factor (mm per pulse):</label><br /> -->
            <input type="text" id="factor_mm" name="factor_mm" value="Default mm value" /><br />
            <!-- <label for="factor_inch">Factor (inch per pulse):</label><br /> -->
            <input type="text" id="factor_inch" name="factor_inch" value="Default inch value" />
            </br>
            </br>
            <button type="button" onclick="updateFactors()">
                Update Factors
            </button>
        </form>
    </div>





    <div class="dro-container" id="container6">
        <button>Process Holes Circle</button>
        <button>Process Holes Line</button>
        <button>R Cut</button>
        <button>Process Slope</button>
    </div>

    <!-- Container 7: Arrow keys -->
    <div class="dro-container" id="container7">
        <div class="arrow-row up">
            <button class="up-arrow"></button>
        </div>
        <div class="arrow-row middle">
            <button class="left-arrow"></button>
            <button class="down-arrow"></button>
            <button class="right-arrow"></button>
        </div>
    </div>






    <!-- Display the value of the calculator -->
    <div class="dro-container" id="container11">
        <div class="readout">
            <span id="calctemp"></span>
        </div>
    </div>

    <!-- Container 3: Calculator grid -->
    <div class="dro-container" id="container8">
        <div class="calculator-grid">

            <button>1/2</button>
            <button id="toggleButton" onclick="toggleMeasureMode()">
                INCH/MM
            </button>
            <!-- <span id="modeMeasureIndicator">INCH</span> -->
            <!-- <p>Position: <span id="poss">0</span></p> -->

            <button id="calculatorModeBtn" onclick="toggleCalculatorMode()">
                Calculator Mode
            </button>
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

            <button id="plusBtn" class="operator" onclick="addOperation('+')">
                PLUS
            </button>
            <button id="multiplyBtn" class="operator" onclick="addOperation('*')">
                MULTI
            </button>
            <button id="subtractBtn" class="operator" onclick="addOperation('-')">
                SUB
            </button>
            <button id="divideBtn" class="operator" onclick="addOperation('/')">
                DIV
            </button>
            <button onclick="calculate()">EQUAL</button>
        </div>
    </div>

    <!-- Container 4: Additional control buttons -->
    <div class="dro-container" id="container9">
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



    <script>
        var currentOperation = null;
        var tempNumber = "";
        var currentValue = 0;
        var calculatorMode = false;

        function updateFactors() {
            var factor_mm = document.getElementById("factor_mm").value;
            var factor_inch = document.getElementById("factor_inch").value;
            fetch("/define-factor", {
                method: "POST",
                headers: {
                    "Content-Type": "application/x-www-form-urlencoded",
                },
                body:
                    "factor_mm=" +
                    encodeURIComponent(factor_mm) +
                    "&factor_inch=" +
                    encodeURIComponent(factor_inch),
            })
                .then((response) => {
                    if (response.ok) {
                        return response.text();
                    } else {
                        throw new Error("Network response was not ok");
                    }
                })
                .then((data) => {
                    document.getElementById("message").innerText = data;
                })
                .catch((error) => {
                    console.error("Error:", error);
                });
        }

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
            } else {
                calculatorModeBtn.classList.remove("calculator-mode-enabled");
                calculatorModeBtn.classList.add("calculator-mode-disabled");
            }
        }

        function updateOperatorButtons() {
            var operatorButtons = document.getElementsByClassName("operator");
            for (var i = 0; i < operatorButtons.length; i++) {
                if (calculatorMode) {
                    operatorButtons[i].removeAttribute("disabled");
                } else {
                    operatorButtons[i].setAttribute("disabled", "disabled");
                }
            }
        }

        function addOperation(operation) {
            if (calculatorMode) {
                if (currentOperation === null) {
                    currentOperation = operation;
                    currentValue = parseFloat(
                        document.getElementById("position").innerText
                    );
                    tempNumber = "";
                }
            }
        }

        function addDecimal() {
            // Ensure tempNumber doesn't already contain a decimal point
            if (!tempNumber.includes(".")) {
                tempNumber += ".";
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
                    case "+":
                        currentValue += parseFloat(tempNumber);
                        break;
                    case "*":
                        currentValue *= parseFloat(tempNumber);
                        break;
                    case "-":
                        currentValue -= parseFloat(tempNumber);
                        break;
                    case "/":
                        var divisor = parseFloat(tempNumber);
                        if (divisor !== 0) {
                            currentValue /= divisor;
                        } else {
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
                .then((response) => {
                    if (!response.ok) {
                        throw new Error("Network response was not ok");
                    }
                    return response.text();
                })
                .then((data) => {
                    console.log(data); // Log the server response for debugging.
                    alert(data); // Alert the user or update the status on the page.
                })
                .catch((error) => {
                    console.error(
                        "There was a problem with the fetch operation:",
                        error
                    );
                    alert("Failed to zero all axes: " + error.message); // Provide error feedback.
                });
        }

        // function toggleMode() {
        //     fetch("/toggle-mode")
        //         .then((response) => response.text())
        //         .then((data) => {
        //             // Now using 'modeIndicator' as the ID for the mode display element
        //             document.getElementById("modeIndicator").innerText = data;
        //             updatePosition(); // Update positions if needed, otherwise you can remove this line         // off for atm
        //         })
        //         .catch(console.error);
        // }

        function toggleMode() {
    fetch("/toggle-mode")
        .then((response) => response.text())
        .then((mode) => {
            document.getElementById("modeIndicator").innerText = mode;
            updatePositions();  // Refresh display to show correct mode values
        })
        .catch(console.error);
}


        function toggleMeasureMode() {
            fetch("/toggle-measure-mode")
                .then((response) => response.text())
                .then((data) => {
                    // Now using 'modeIndicator' as the ID for the mode display element
                    document.getElementById("modeMeasureIndicator").innerText = data;
                    // updatePosition(); // Update positions if needed, otherwise you can remove this line
                })
                .catch(console.error);
        }

        // function updatePosition() {
        //     fetch("/get-positions")
        //         .then((response) => response.json())
        //         .then((data) => {
        //             document.getElementById("position").innerText = data.positionX;
        //             document.getElementById("position2").innerText = data.positionY;
        //             document.getElementById("position3").innerText = data.positionZ;
        //         })
        //         .catch(console.error);
        // }

            function updatePosition() {
    fetch("/get-positions")
        .then((response) => {
            if (!response.ok) throw new Error("Failed to fetch positions");
            return response.json();
        })
        .then((data) => {
            console.log("Received position data:", data); // Debug: log received data
            document.getElementById("position").innerText = data.positionX;
            document.getElementById("position2").innerText = data.positionY;
            document.getElementById("position3").innerText = data.positionZ;
        })
        .catch((error) => {
            console.error("Error fetching positions:", error);
        });
}

        // // Call updatePosition at an interval
        // setInterval(updatePosition, 100);  // Update every second

        // function updatePositions() {
        //     fetch("/position")
        //         .then((response) => response.text())
        //         .then(
        //             (data) => (document.getElementById("position").innerText = data)
        //         )
        //         .catch(console.error);

        //     fetch("/position2")
        //         .then((response) => response.text())
        //         .then(
        //             (data) => (document.getElementById("position2").innerText = data)
        //         )
        //         .catch(console.error);

        //     fetch("/position3")
        //         .then((response) => response.text())
        //         .then(
        //             (data) => (document.getElementById("position3").innerText = data)
        //         )
        //         .catch(console.error);
        // }

function updatePositions() {
    fetch("/get-positions")
        .then(response => response.json())
        .then(data => {
            const mode = document.getElementById("modeIndicator").innerText;
            document.getElementById("position").innerText = (mode === "ABS" && data.positionX_ABS !== undefined) ? data.positionX_ABS : data.positionX_INC;
            document.getElementById("position2").innerText = (mode === "ABS" && data.positionY_ABS !== undefined) ? data.positionY_ABS : data.positionY_INC;
            document.getElementById("position3").innerText = (mode === "ABS" && data.positionZ_ABS !== undefined) ? data.positionZ_ABS : data.positionZ_INC;
        })
        .catch(error => {
            console.error('Error fetching position data:', error);
            // Optionally set a default or placeholder text if there is an error
            document.getElementById("position").innerText = "N/A";
            document.getElementById("position2").innerText = "N/A";
            document.getElementById("position3").innerText = "N/A";
        });
}


        function resetPosition(axis) {
            fetch("/reset/" + axis)
                .then((response) => {
                    if (response.ok) {
                        console.log(axis.toUpperCase() + " position reset"); // print message
                        // updatePositions(); // Refresh the positions immediately
                    }
                })
                .catch(console.error);
        }

        function updatePlaneDisplay() {
            fetch("/get-current-plane")
                .then((response) => response.text())
                .then((data) => {
                    console.log("Updated Plane Index: ", data); // Log for debugging
                    document.getElementById("currentPlane").innerText =
                        "Plane: " + data;
                })
                .catch((error) =>
                    console.error("Failed to update plane display:", error)
                );
        }

        function updatePositionsAndPlane() {
            updatePosition();
            updatePlaneDisplay();
        }

        function savePoint() {
            const x = document.getElementById("xInput").value;
            const y = document.getElementById("yInput").value;
            const z = document.getElementById("zInput").value;

            fetch(`/add-point?x=${x}&y=${y}&z=${z}`, { method: "POST" })
                .then((response) => response.text())
                .then((data) => {
                    alert("Point saved");
                    getLastPoint(); // Fetch the last point after saving
                })
                .catch((error) => console.error("Error saving point:", error));
        }

        function getLastPoint() {
            fetch("/get-last-point")
                .then((response) => response.text())
                .then((data) => {
                    document.getElementById("lastPoint").innerText =
                        "Last Point: " + data;
                })
                .catch((error) => {
                    console.error("Error fetching last point:", error);
                    document.getElementById("lastPoint").innerText =
                        "Last Point: Error";
                });
        }

        function getAllPoints() {
            fetch("/get-all-points")
                .then((response) => {
                    if (!response.ok) throw new Error("No points found");
                    return response.json();
                })
                .then((data) => {
                    const pointsList = document.getElementById("pointsList");
                    pointsList.innerHTML = ""; // Clear existing points

                    data.points.forEach((point, index) => {
                        const pointItem = document.createElement("button");
                        pointItem.textContent = `Point ${index + 1}: (${point.x}, ${point.y
                            }, ${point.z})`;
                        pointItem.addEventListener("click", () => {
                            displayPoint(point);
                        });
                        pointsList.appendChild(pointItem);
                    });
                })
                .catch((error) => {
                    console.error("Error fetching all points:", error);
                    document.getElementById("pointsList").innerText =
                        "Failed to load points";
                });
        }

        function displayPoint(point) {
            document.getElementById("position").textContent = point.x;
            document.getElementById("position2").textContent = point.y;
            document.getElementById("position3").textContent = point.z;
        }

        function saveCurrentPosition() {
            fetch("/save-current-position")
                .then((response) => {
                    if (response.ok) {
                        return response.text();
                    }
                    throw new Error("Could not save position");
                })
                .then((message) => {
                    console.log(message);
                    getAllPoints(); // Refresh the points display
                })
                .catch((error) => console.error("Error:", error));

            const canvas = document.getElementById('coordinateCanvas');

            const ctx = canvas.getContext('2d');

            function drawGrid() {
                const width = canvas.width;
                const height = canvas.height;
                ctx.beginPath();
                ctx.strokeStyle = '#ccc';

                // Draw grid lines every 50 pixels
                for (let x = 0; x <= width; x += 50) {
                    ctx.moveTo(x, 0);
                    ctx.lineTo(x, height);
                }
                for (let y = 0; y <= height; y += 50) {
                    ctx.moveTo(0, y);
                    ctx.lineTo(width, y);
                }
                ctx.stroke();
            }

            function drawPoints(points) {
                ctx.fillStyle = 'red';
                points.forEach(point => {
                    ctx.beginPath();
                    ctx.arc(point.x, point.y, 5, 0, Math.PI * 2, true); // Draw circle for each point
                    ctx.fill();
                });
            }

            drawGrid();

        }

        setInterval(updatePositionsAndPlane, 1000); // Adjust interval to 1000 ms
        setInterval(updatePositions, 50); // Call updatePositions() every 1000ms (1 second) but right now it is 50ms so stupid fast
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

    // server.on("/get-positions", HTTP_GET, [](AsyncWebServerRequest *request)
    //           {
    //               char formattedX[20], formattedY[20], formattedZ[20];

    //               // Assuming encoder1, encoder2, encoder3 are your encoder instances
    //               String positionX = formatPosition(isABSMode ? encoder1.position - planes[currentPlaneIndex].encoderValueABS[0] : encoder1.position - planes[currentPlaneIndex].last_INC[0], isInchMode);
    //               String positionY = formatPosition(isABSMode ? encoder2.position - planes[currentPlaneIndex].encoderValueABS[1] : encoder2.position - planes[currentPlaneIndex].last_INC[1], isInchMode);
    //               String positionZ = formatPosition(isABSMode ? encoder3.position - planes[currentPlaneIndex].encoderValueABS[2] : encoder3.position - planes[currentPlaneIndex].last_INC[2], isInchMode);

    //               // Create a JSON object to send the formatted positions
    //               StaticJsonDocument<200> jsonDoc;
    //               jsonDoc["positionX"] = positionX;
    //               jsonDoc["positionY"] = positionY;
    //               jsonDoc["positionZ"] = positionZ;

    //               String jsonString;
    //               serializeJson(jsonDoc, jsonString);

    //               request->send(200, "application/json", jsonString); // Send JSON data
    //           });

// server.on("/get-positions", HTTP_GET, [](AsyncWebServerRequest *request) {
//     String positionX = formatPosition(isABSMode ? encoder1.position - planes[currentPlaneIndex].encoderValueABS[0] : encoder1.position - planes[currentPlaneIndex].last_INC[0], isInchMode);
//     String positionY = formatPosition(isABSMode ? encoder2.position - planes[currentPlaneIndex].encoderValueABS[1] : encoder2.position - planes[currentPlaneIndex].last_INC[1], isInchMode);
//     String positionZ = formatPosition(isABSMode ? encoder3.position - planes[currentPlaneIndex].encoderValueABS[2] : encoder3.position - planes[currentPlaneIndex].last_INC[2], isInchMode);

//     StaticJsonDocument<200> jsonDoc;
//     jsonDoc["positionX"] = positionX;
//     jsonDoc["positionY"] = positionY;
//     jsonDoc["positionZ"] = positionZ;

//     Serial.println("Mode: " + String(isABSMode ? "ABS" : "INC"));
//     Serial.println("Position X: " + String(positionX));
//     Serial.println("Position Y: " + String(positionY));
//     Serial.println("Position Z: " + String(positionZ));

//     String jsonString;
//     serializeJson(jsonDoc, jsonString);
//     request->send(200, "application/json", jsonString);
// });

server.on("/get-positions", HTTP_GET, [](AsyncWebServerRequest *request)
{
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["positionX_ABS"] = formatPosition(encoder1.position - planes[currentPlaneIndex].last_ABS[0], isInchMode);
    jsonDoc["positionY_ABS"] = formatPosition(encoder2.position - planes[currentPlaneIndex].last_ABS[1], isInchMode);
    jsonDoc["positionZ_ABS"] = formatPosition(encoder3.position - planes[currentPlaneIndex].last_ABS[2], isInchMode);

    jsonDoc["positionX_INC"] = formatPosition(encoder1.position - planes[currentPlaneIndex].last_INC[0], isInchMode);
    jsonDoc["positionY_INC"] = formatPosition(encoder2.position - planes[currentPlaneIndex].last_INC[1], isInchMode);
    jsonDoc["positionZ_INC"] = formatPosition(encoder3.position - planes[currentPlaneIndex].last_INC[2], isInchMode);

    String jsonString;
    serializeJson(jsonDoc, jsonString);
    request->send(200, "application/json", jsonString);
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
                request->send(200, "text/plain", "All positions reset"); });

    server.on("/get-current-plane", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/plain", String(currentPlaneIndex + 1)); // +1 to make it human-readable (1-based index)
              });

    // Endpoint to add a point to the current plane
    server.on("/add-point", HTTP_POST, [](AsyncWebServerRequest *request)
              {
    int x = 0, y = 0, z = 0;
    if (request->hasParam("x") && request->hasParam("y") && request->hasParam("z")) {
        x = request->getParam("x")->value().toInt();
        y = request->getParam("y")->value().toInt();
        z = request->getParam("z")->value().toInt();
        planes[currentPlaneIndex].shapePoints.emplace_back(x, y, z);
        request->send(200, "text/plain", "Point added");
    } else {
        request->send(400, "text/plain", "Missing parameters");
    } });

    server.on("/define-factor", HTTP_POST, [](AsyncWebServerRequest *request)
              {
  if(request->hasParam("factor_mm", true) && request->hasParam("factor_inch", true)){
    float new_factor_mm = request->getParam("factor_mm", true) ->value().toFloat();
    float new_factor_inch = request->getParam("factor_inch", true) ->value().toFloat();
  factor_mm = new_factor_mm;
  factor_inch = new_factor_inch;
  request->send(200, "text/plain", "Factor updated succesfully");
  } else {
      request->send(400, "text/plain", "Missing parameter");
  } });

    // Endpoint to get the last saved point on the current plane
    server.on("/get-last-point", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    if (!planes[currentPlaneIndex].shapePoints.empty()) {
        Point lastPoint = planes[currentPlaneIndex].shapePoints.back();
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d,%d,%d", lastPoint.x, lastPoint.y, lastPoint.z);
        request->send(200, "text/plain", buffer);
    } else {
        request->send(404, "text/plain", "No points saved");
    } });

    // Endpoint to get all saved points on the current plane
    server.on("/get-all-points", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      if (!planes[currentPlaneIndex].shapePoints.empty()) {
          String jsonOutput;
          StaticJsonDocument<1024> doc;  // Adjust size as needed based on expected data volume
          JsonArray pointsArray = doc.createNestedArray("points");

          for (const auto& point : planes[currentPlaneIndex].shapePoints) {
              JsonObject pointObj = pointsArray.createNestedObject();
              pointObj["x"] = point.x;
              pointObj["y"] = point.y;
              pointObj["z"] = point.z;
          }

          serializeJson(doc, jsonOutput);
          request->send(200, "application/json", jsonOutput);
      } else {
          request->send(404, "text/plain", "No points saved");
      } });

    server.on("/save-current-position", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    addCurrentPositionToPoint();  // Call the function to add the point
    request->send(200, "text/plain", "Current position saved"); });

    server.begin();

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

void updateDisplayContent()
{
    char buffer[128]; // Make sure the buffer is large enough to hold the string
    int xOffset;      // Horizontal offset to right-align text

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
        // // Display the X and Y axes for the two-axis mode
        // displayAxisValues(0, 0); // X-axis
        // displayAxisValues(1, 16); // Y-axis
        refreshAndDrawPoints();
        drawGrid();
        LCDTextDraw(0, 50, "> return ", 1, WHITE, BLACK); // Return option
        break;

    case THREE_AXIS:
        // Display the X, Y, and Z axes for the three-axis mode including the plane index
        snprintf(buffer, sizeof(buffer), "Plane %d - %s Mode", currentPlaneIndex + 1, isABSMode ? "ABS" : "INC");
        LCDTextDraw(0, 0, buffer, 1, WHITE, BLACK);       // Display the plane and mode at the top
        displayAxisValues(0, 16);                         // X-axis
        displayAxisValues(1, 32);                         // Y-axis
        displayAxisValues(2, 48);                         // Z-axis
        LCDTextDraw(0, 64, "> return ", 1, WHITE, BLACK); // Return option
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
