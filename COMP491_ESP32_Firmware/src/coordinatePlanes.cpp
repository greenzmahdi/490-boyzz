#include "coordinatePlanes.h"
#include "oled.h"
#include "encoder.h"
#include "format.h"

int currentPlaneIndex = 0; // Keep track of the current plane index
bool isABSMode = true;     // Start in ABS mode

CoordinatePlane planes[12];

void toggleMode()
{
    isABSMode = !isABSMode;
}

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

void resetEncoderValue(int encoderIndex)
{
    if (encoderIndex < 0 || encoderIndex >= 3)
    {
        Serial.println("Error: Encoder index out of range");
        return;
    }

    // Get references to the current plane for easier access
    auto &currentPlane = planes[currentPlaneIndex];
    auto &lastABS = currentPlane.last_ABS;
    auto &lastINC = currentPlane.last_INC;
    auto &encoderValueABS = currentPlane.encoderValueABS;
    auto &encoderValueINC = currentPlane.encoderValueINC;

    Encoder &encoder = (encoderIndex == 0) ? encoder1 : (encoderIndex == 1) ? encoder2
                                                                            : encoder3;

    if (isABSMode)
    {
        encoderValueABS[encoderIndex] = 0;        // Zero out the ABS value
        lastABS[encoderIndex] = encoder.position; // Store last ABS position
    }
    else
    {
        encoderValueINC[encoderIndex] = 0;        // Zero out the INC value
        lastINC[encoderIndex] = encoder.position; // Store last INC position
    }
}

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

void addPointToCurrentPlane(int x, int y, int z)
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