#include "buttons.h"

#include "I2C.h"
// BUTTON FUNCTIONS //

bool ButtonRead(int idx)
{
    // 0 - left
    // 1 - center
    // 2 - up
    // 3 - down
    // 4 - right

    if ((idx < 0) || (idx > 4))
        return false;

    return !I2CReadReg(0x20, 1, idx);
}

bool ButtonLeftPressed()
{
    return ButtonRead(0);
}

bool ButtonCenterPressed()
{
    return ButtonRead(1);
}

bool ButtonUpPressed()
{
    return ButtonRead(2);
}

bool ButtonDownPressed()
{
    return ButtonRead(3);
}

bool ButtonRightPressed()
{
    return ButtonRead(4);
}

bool ButtonStatesPrev[] = {false, false, false, false, false};

bool stateButtonCenter = ButtonCenterPressed();
bool stateButtonUp = ButtonUpPressed();
bool stateButtonDown = ButtonDownPressed();
bool stateButtonLeft = ButtonLeftPressed();
bool stateButtonRight = ButtonRightPressed();

void updateButtonStates()
{
    ButtonStatesPrev[0] = ButtonLeftPressed();
    ButtonStatesPrev[1] = ButtonCenterPressed();
    ButtonStatesPrev[2] = ButtonUpPressed();
    ButtonStatesPrev[3] = ButtonDownPressed();
    ButtonStatesPrev[4] = ButtonRightPressed();
}