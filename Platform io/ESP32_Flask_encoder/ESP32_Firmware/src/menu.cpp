#include "menu.h"

volatile MenuState currentMenuState = MAIN_MENU;
volatile int menuItemIndex = 0; // Index of the selected menu item

// Menu Options
const char *MenuOptions[] = {"Connect Online", "Connect Offline"}; // might not need this, depends on design
const char *MenuDroItems[] = {"2-Axis", "3-Axis"};
const char *SinoAxis[] = {"X: ", "Y: "};
const char *ToAutoAxis[] = {"X: ", "Y: ", "Z: "};