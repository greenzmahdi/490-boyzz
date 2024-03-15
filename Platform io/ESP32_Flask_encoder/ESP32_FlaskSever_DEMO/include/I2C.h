// #ifndef I2C_SETUP_H
// #define I2C_SETUP_H

// #include <WiFi.h>

// enum MenuState;

// volatile MenuState currentMenuState;
// volatile int menuItemIndex; // Index of the selected menu item


// // I2C FUNCTIONS //
// byte I2CReadRegs(int address, int size);

// bool I2CReadReg(int address, int size, int idx);

// void I2CWriteReg(int address, int pin, bool state);

// // monitor z pin state //
// bool ZPinState(int idx);

// bool pinZ1State();

// bool pinZ2State();

// bool pinZ3State();

// bool pinZ4State();

// bool pinZ5State();

// bool PinStatePrev[] = {false, true, false, false, false};

// bool PinStateZ1;
// bool PinStateZ2;
// bool PinStateZ3;
// bool PinStateZ4;
// bool PinStateZ5;

// void updateAllPinZ();

// bool updateZPinState(int pinAIdx, int pinBIdx, int pinZIdx);

// // BUTTON FUNCTIONS //

// bool ButtonRead(int idx);

// bool ButtonLeftPressed();

// bool ButtonCenterPressed();

// bool ButtonUpPressed();

// bool ButtonDownPressed();

// bool ButtonRightPressed();

// bool ButtonStatesPrev[] = {false, false, false, false, false};

// bool stateButtonCenter;
// bool stateButtonUp;
// bool stateButtonDown;
// bool stateButtonLeft;
// bool stateButtonRight;

// void updateButtonStates();

// void handleMenuNavigation();

// #endif