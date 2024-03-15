// #include <Wire.h>
// #include "encoder.h"
// #include "oled.h"


// enum MenuState
// {
//   MAIN_MENU,
//   TWO_AXIS,
//   THREE_AXIS
// };

// volatile MenuState currentMenuState = MAIN_MENU;
// volatile int menuItemIndex = 0; // Index of the selected menu item


// // I2C FUNCTIONS //
// byte I2CReadRegs(int address, int size)
// {
//   Wire.beginTransmission(address);
//   Wire.write(0x00);
//   Wire.endTransmission();

//   Wire.requestFrom(address, size);

//   return Wire.read();
// }

// bool I2CReadReg(int address, int size, int idx)
// {
//   byte regs = I2CReadRegs(address, size);

//   return bitRead(regs, idx);
// }

// void I2CWriteReg(int address, int pin, bool state)
// {
//   // This function should write 'state' to 'pin' at 'address' on your I2C expander
//   // The implementation details will vary depending on your specific I2C expander chip

//   byte dataToWrite;

//   // Example: If you need to write a single bit, you'll likely first read the current state
//   // of all pins, modify the bit for 'pin' to 'state', and then write back.
//   Wire.beginTransmission(address);
//   Wire.write(0x00); // Assuming the register you're writing to, often the GPIO register
//   if (state)
//   {
//     dataToWrite |= (1 << pin); // Set the bit for the pin
//   }
//   else
//   {
//     dataToWrite &= ~(1 << pin); // Clear the bit for the pin
//   }
//   Wire.write(dataToWrite);
//   Wire.endTransmission();
// }

// // monitor z pin state //
// bool ZPinState(int idx)
// {
//   // PIN Z1 = 9
//   // PIN Z2 = 10
//   // PIN Z3 = 11
//   // PIN Z4 = 12
//   // PIN Z5 = 13

//   if ((idx < 9) || (idx > 14))
//     return false;

//   return !I2CReadReg(0x20, 1, idx);
// }

// bool pinZ1State()
// {
//   return ZPinState(9);
// }

// bool pinZ2State()
// {
//   return ZPinState(10);
// }

// bool pinZ3State()
// {
//   return ZPinState(11);
// }

// bool pinZ4State()
// {
//   return ZPinState(12);
// }

// bool pinZ5State()
// {
//   return ZPinState(13);
// }

// bool PinStatePrev[] = {false, true, false, false, false};

// bool PinStateZ1 = pinZ1State();
// bool PinStateZ2 = pinZ2State();
// bool PinStateZ3 = pinZ3State();
// bool PinStateZ4 = pinZ4State();
// bool PinStateZ5 = pinZ5State();

// void updateAllPinZ()
// {
//   PinStatePrev[0] = pinZ1State();
//   PinStatePrev[1] = pinZ2State();
//   PinStatePrev[2] = pinZ3State();
//   PinStatePrev[3] = pinZ4State();
//   PinStatePrev[4] = pinZ5State();
// }

// bool updateZPinState(int pinAIdx, int pinBIdx, int pinZIdx)
// {
//   bool stateA = I2CReadReg(0x20, 1, pinAIdx); // Read state of pin A
//   bool stateB = I2CReadReg(0x20, 1, pinBIdx); // Read state of pin B

//   bool zState = stateA && stateB; // Z is HIGH if both A and B are HIGH

//   I2CWriteReg(0x20, pinZIdx, zState); // Update Z pin state

//   return zState;
// }

// // BUTTON FUNCTIONS //

// bool ButtonRead(int idx)
// {
//   // 0 - left
//   // 1 - center
//   // 2 - up
//   // 3 - down
//   // 4 - right

//   if ((idx < 0) || (idx > 4))
//     return false;

//   return !I2CReadReg(0x20, 1, idx);
// }

// bool ButtonLeftPressed()
// {
//   return ButtonRead(0);
// }

// bool ButtonCenterPressed()
// {
//   return ButtonRead(1);
// }

// bool ButtonUpPressed()
// {
//   return ButtonRead(2);
// }

// bool ButtonDownPressed()
// {
//   return ButtonRead(3);
// }

// bool ButtonRightPressed()
// {
//   return ButtonRead(4);
// }

// bool ButtonStatesPrev[] = {false, false, false, false, false};

// bool stateButtonCenter = ButtonCenterPressed();
// bool stateButtonUp = ButtonUpPressed();
// bool stateButtonDown = ButtonDownPressed();
// bool stateButtonLeft = ButtonLeftPressed();
// bool stateButtonRight = ButtonRightPressed();

// void updateButtonStates()
// {
//   // ButtonStatesPrev[0] = ButtonLeftPressed();
//   // ButtonStatesPrev[1] = ButtonCenterPressed();
//   // ButtonStatesPrev[2] = ButtonUpPressed();
//   // ButtonStatesPrev[3] = ButtonDownPressed();
//   // ButtonStatesPrev[4] = ButtonRightPressed();

//   ButtonStatesPrev[0] = stateButtonCenter;
//   ButtonStatesPrev[1] = stateButtonUp;
//   ButtonStatesPrev[2] = stateButtonDown;
//   ButtonStatesPrev[3] = stateButtonLeft;
//   ButtonStatesPrev[4] = stateButtonRight;
// }

// void handleMenuNavigation()
// {
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   if (ButtonUpPressed() && !ButtonStatesPrev[2])
//   {
//     // ensures curr state is in MAIN_MENU, ensuring it does not go below 0
//     if (currentMenuState == MAIN_MENU)
//     {
//       menuItemIndex = max(0, menuItemIndex - 1);
//     }
//   }
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   else if (ButtonDownPressed() && !ButtonStatesPrev[3])
//   {
//     if (currentMenuState == MAIN_MENU)
//     {
//       menuItemIndex = min(1, menuItemIndex + 1); // For now we just have two menu options
//       // menuItemIndex = min(2, menuItemIndex + 1); // Assuming we want to add 3 menu items (in the case we want to add another option)
//     }
//   }
//   // check if button being pressed is diff from its last prev state aka (true != false)
//   else if (ButtonCenterPressed() && !ButtonStatesPrev[1])
//   {
//     if (currentMenuState == MAIN_MENU)
//     {
//       // based on the state of our menu option, we update our screen with the correct screen
//       // we clear the screen and update display
//       switch (menuItemIndex)
//       {
//       case 0:
//         LCDScreenClear();
//         currentMenuState = TWO_AXIS;
//         break;
//       case 1:
//         LCDScreenClear();
//         currentMenuState = THREE_AXIS;
//         break;
//       }
//     }
//     else
//     {
//       currentMenuState = MAIN_MENU; // Allow going back to the main menu
//       LCDScreenClear();
//     }
//   }
//   // Update previous button states at the end of your button handling logic
//   ButtonStatesPrev[0] = stateButtonCenter;
//   ButtonStatesPrev[1] = stateButtonUp;
//   ButtonStatesPrev[2] = stateButtonDown;
//   ButtonStatesPrev[3] = stateButtonLeft;
//   ButtonStatesPrev[4] = stateButtonRight;

//   updateButtonStates();
//   // updateAllPinZ(); // update all
// }