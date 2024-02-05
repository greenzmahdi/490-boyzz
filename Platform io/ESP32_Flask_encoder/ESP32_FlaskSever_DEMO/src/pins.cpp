#include <Wire.h>
#include "pin_setup.h"

// Pin func setup
void setUpPins()
{
    // Setting OLED PINs
    pinMode(PIN_I2C_SCL, OUTPUT);
    pinMode(PIN_I2C_SDA, OUTPUT);

    pinMode(PIN_LED, OUTPUT);
    pinMode(PIN_A1, INPUT);
    pinMode(PIN_A2, INPUT);
    pinMode(PIN_A3, INPUT);
    pinMode(PIN_A4, INPUT);
    pinMode(PIN_A5, INPUT);
    pinMode(PIN_A6, INPUT);
    pinMode(PIN_B1, INPUT);
    pinMode(PIN_B2, INPUT);
    pinMode(PIN_B3, INPUT);
    pinMode(PIN_B4, INPUT);
    pinMode(PIN_B5, INPUT);
    pinMode(PIN_B6, INPUT);

    pinMode(PIN_PROBE, INPUT);
    pinMode(PIN_TACH, INPUT);
    pinMode(PIN_IOINT ,INPUT);

};