// #ifndef PINS_H
// #define PINS_H

// // PIN SETUP //

// // Pin (A & B)
// const int PIN_A1;
// const int PIN_A2;
// const int PIN_A3;
// const int PIN_A4;
// const int PIN_A5;
// const int PIN_A6;
// const int PIN_B1;
// const int PIN_B2;
// const int PIN_B3;
// const int PIN_B4;
// const int PIN_B5;
// const int PIN_B6;

// // Led pin
// extern const int PIN_LED = 12;

// // OLED pins
// extern const int PIN_I2C_SCL = 16;
// extern const int PIN_I2C_SDA = 13;

// // Function declarations 
// void setUpPins();

// #endif



// pin_setup.h
#ifndef PIN_SETUP_H
#define PIN_SETUP_H

// Pin (A & B)
const int PIN_A1 = 33;
const int PIN_A2 = 32;
const int PIN_A3 = 35;
const int PIN_A4 = 34;
const int PIN_A5 = 18;
const int PIN_A6 = 27;
const int PIN_B1 = 26;
const int PIN_B2 = 4;
const int PIN_B3 = 17;
const int PIN_B4 = 22;
const int PIN_B5 = 23;
const int PIN_B6 = 14;

// Led pin
const int PIN_LED = 12;

// OLED pins
const int PIN_I2C_SCL = 16;
const int PIN_I2C_SDA = 13;

// Function declaration
void setUpPins();

#endif
