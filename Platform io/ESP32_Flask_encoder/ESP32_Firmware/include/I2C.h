#ifndef I2C_SETUP_H
#define I2C_SETUP_H

#include <WiFi.h>

// I2C FUNCTIONS //
byte I2CReadRegs(int address, int size);

bool I2CReadReg(int address, int size, int idx);

void I2CWriteReg(int address, int pin, bool state);


#endif