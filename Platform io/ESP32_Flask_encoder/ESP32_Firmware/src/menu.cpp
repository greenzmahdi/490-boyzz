#include <math.h>
#include "menu.h"
#include "buttons.h"
#include "oled.h"
#include "coordinatePlanes.h"



volatile MenuState currentMenuState = MAIN_MENU;
volatile int menuItemIndex = 0; // Index of the selected menu item

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"}; // might not need this, depends on design
const char *MenuDroItems[] = {"Coordinate Plane", "DRO"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};

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