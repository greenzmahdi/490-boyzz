#ifndef FORMAT_SETUP_H
#define FORMAT_SETUP_H

#include <Arduino.h>


// Mode selector variable
extern bool isInchMode;

// Inch and milimeter factors
extern float factor_mm;        
extern float factor_inch; 

// Function to format position to fixed decimal places
String formatPosition(float pulses, bool isInchMode);

void toggleMeasurementMode();
#endif