# ESP32 DRO Project

## Overview
This project is a collaborative effort by Randy, Gabe, and Mahdi at CSUN during Fall 2023 and Spring 2024, under the guidance of Professor Jeffrey Wiegley. It aims to create a Digital Readout (DRO) system using an ESP32 microcontroller. This system is designed to enhance lathe operations with precise measurement capabilities, supplemented by various sensors and a user-friendly web interface.

## Features
- Real-time measurement of lathe operations across three axes.
- User-friendly GUI inspired by TOAUTO Digital Readouts, featuring:
  - Display for 3-axis values.
  - Zeroing functionality for individual or all axes.
  - Adjustable precision settings.
  - Coordinate saving and recalling.
  - Built-in calculator for quick on-the-job calculations.
- Wireless connectivity allowing remote monitoring and control via a web interface.

## Design of GUI

The GUI emulates an actual TOAUTO Digital Readout, including features essential for precise machining tasks. It updates the encoder or lathe's position at regular intervals through GET requests to the ESP32. The interface includes:
- Plane indicators for up to 12 different planes.
- ABS/INC toggle for absolute or relative coordinate display.
- INCH/MM toggle for unit conversion.
- Functional buttons (Xo, Yo, Zo, XYZo) for zeroing coordinates of the selected plane.
- A section to define and adjust multiplicative factors for measurement precision.
- Memory functionality to save and recall specific coordinates directly from the GUI.

## Tools, Libraries, Dependencies
The project utilizes PlatformIO for efficient library management and firmware development. Key libraries include:
- **ESPAsyncWebServer & AsyncTCP**: For web server functionality, allowing the ESP32 to serve a web interface.
- **WiFi**: To set up the ESP32 as a wireless access point.
- **Adafruit GFX Library & Adafruit SSD1306**: For OLED display operations.
- **FastLED**: Initially used for learning LED controls, useful for hardware control education.
- **HTTPClient**: For handling HTTP requests.

These libraries are managed via PlatformIO, ensuring easy updates and configuration. For detailed library versions and settings, refer to the `platformio.ini` file.


## Challenges
Challenges faced during development included:
- **WiFi Connectivity**: Establishing reliable communication between the ESP32 and client devices, especially on varied network infrastructures.
- **Encoder Accuracy**: Ensuring the encoder readings accurately reflect on the web interface, particularly when switching between absolute and relative measurements in any coordinate plane.
- **User Interface Design**: Balancing functionality and simplicity in the GUI to cater to both novice and experienced users.

## Future Features
Planned enhancements include:
- **Plane Assignment**: Adding functionality for users to assign and switch between different coordinate planes.
- **Memory Buttons**: Implementing functionality for the saved coordinate buttons to load their respective values into the DRO.
- **Network Connectivity**: Enabling internet access through the ESP32's WiFi network to facilitate easier testing and updates.


## Testing
Perform a test run to verify that the system functions as expected.
Access the ESP32 web server using a web browser by connecting to the ESP32’s Wi-Fi network and navigating to 192.168.4.1.

## Contributors
- **Gabriel Sosa** 
- **Mahdi Mazloumi** 
- **Randy Herrera** 

## Acknowledgments
Special thanks to Professor Jeffrey Wiegley for his guidance and support throughout the project and providing the resources necessary for this project.
