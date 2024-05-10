#ifndef COORDINATE_PLANES_SETUP_H
#define COORDINATE_PLANES_SETUP_H

#include <vector>

extern int currentPlaneIndex; // Keep track of the current plane index
extern bool isABSMode;     // Start in ABS mode

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

extern CoordinatePlane planes[12];

void selectPlane(int index);

void nextPlane();
void previousPlane();
void addCurrentPositionToPoint();
void resetEncoderValue(int encoderIndex);
// Helper function to format and display axis values based on current settings
void displayAxisValues(int axis, int yPosition);
void toggleMode();

void updateDisplayWithPoints();

void refreshAndDrawPoints();

void addPointToCurrentPlane(int x, int y, int z);

// Functions have not been implemented //
void removeLastPointFromCurrentPlane(int planeIndex);

void displayCurrentPoints(int planeIndex);

void clearPointsInPlane(int planeIndex);
#endif