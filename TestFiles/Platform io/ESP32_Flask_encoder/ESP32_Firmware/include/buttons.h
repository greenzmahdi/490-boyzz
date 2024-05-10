#ifndef BUTTONS_SETUP_H
#define BUTTONS_SETUP_H

#include "I2C.h"

// BUTTON FUNCTIONS //

bool ButtonRead(int idx);

bool ButtonLeftPressed();

bool ButtonCenterPressed();

bool ButtonUpPressed();

bool ButtonDownPressed();

bool ButtonRightPressed();

extern bool ButtonStatesPrev[];

void updateButtonStates();
#endif