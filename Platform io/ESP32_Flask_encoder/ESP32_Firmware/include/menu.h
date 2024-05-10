#ifndef MENU_SETUP_H
#define MENU_SETUP_H

enum MenuState
{
    MAIN_MENU,
    TWO_AXIS,
    THREE_AXIS,
    SHAPE_CREATION
};

extern volatile MenuState currentMenuState;
extern volatile int menuItemIndex; // Index of the selected menu item

// Menu Options
extern const char *MenuOptions[]; 
extern const char *MenuDroItems[];
extern const char *SinoAxis[];
extern const char *ToAutoAxis[];

void handleMenuNavigation();
void updateDisplayContent();
#endif