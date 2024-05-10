#include "format.h"

// Mode selector variable
bool isInchMode = true;

// Inch and milimeter factors
float factor_mm = 0.01;        // 0.01 mm per pulse
float factor_inch = 0.0003937; // Accurate conversion to maintain equivalence

void toggleMeasurementMode()
{
    isInchMode = !isInchMode;
}

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