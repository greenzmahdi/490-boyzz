#ifndef COORDINATE_PLANES_SETUP_H
#define COORDINATE_PLANES_SETUP_H

#include <vector>

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


#endif