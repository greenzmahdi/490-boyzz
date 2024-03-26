#include <Wire.h>
#include "pin.h"

// Pin func setup
void setUpPins()
{
    // Setting OLED PINs
    pinMode(PIN_I2C_SCL, OUTPUT);
    pinMode(PIN_I2C_SDA, OUTPUT);

    pinMode(PIN_LED, OUTPUT);
    
    pinMode(PIN_A1, INPUT_PULLUP);
    pinMode(PIN_A2, INPUT_PULLUP);
    pinMode(PIN_A3, INPUT_PULLUP);
    pinMode(PIN_A4, INPUT_PULLUP);
    pinMode(PIN_A5, INPUT_PULLUP);
    pinMode(PIN_A6, INPUT_PULLUP);
    pinMode(PIN_B1, INPUT_PULLUP);
    pinMode(PIN_B2, INPUT_PULLUP);
    pinMode(PIN_B3, INPUT_PULLUP);
    pinMode(PIN_B4, INPUT_PULLUP);
    pinMode(PIN_B5, INPUT_PULLUP);
    pinMode(PIN_B6, INPUT_PULLUP);

    pinMode(PIN_PROBE, INPUT);
    pinMode(PIN_TACH, INPUT);
    pinMode(PIN_IOINT ,INPUT_PULLUP);

};