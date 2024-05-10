#include<Wire.h>
#include "I2C.h"

// I2C FUNCTIONS //
byte I2CReadRegs(int address, int size)
{
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(address, size);

  return Wire.read();
}

bool I2CReadReg(int address, int size, int idx)
{
  byte regs = I2CReadRegs(address, size);

  return bitRead(regs, idx);
}
// ATM not being used, alternarively I believe we can use Arduino.h -> bitSet() / 
void I2CWriteReg(int address, int pin, bool state)
{
  // This function should write 'state' to 'pin' at 'address' on your I2C expander
  // The implementation details will vary depending on your specific I2C expander chip

  byte dataToWrite;

  // Example: If you need to write a single bit, you'll likely first read the current state
  // of all pins, modify the bit for 'pin' to 'state', and then write back.
  Wire.beginTransmission(address);
  Wire.write(0x00); // Assuming the register you're writing to, often the GPIO register
  if (state)
  {
    dataToWrite |= (1 << pin); // Set the bit for the pin
  }
  else
  {
    dataToWrite &= ~(1 << pin); // Clear the bit for the pin
  }
  Wire.write(dataToWrite);
  Wire.endTransmission();
}