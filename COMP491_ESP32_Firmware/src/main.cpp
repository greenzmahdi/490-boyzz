// file imports
#include "pin.h"
#include "led.h"
#include "oled.h"
#include "encoder.h"
#include "menu.h"
#include "networkConfig.h"

// Forward declarations
void TaskUpdateDisplay(void *pvParameters);

void setup()
{
    Serial.begin(115200);
    delay(10);
    
    // PIN SETUP
    setUpPins();

    // setting up LEDs
    turnOffLEDs();
    
    // Init OLED
    Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.begin();
    Wire.setClock(400000);
    LCDInit();
    LCDScreenClear();

    // Set up WiFi access point
    setupAccessPoint(); 

    // Set up web server routes
    setupWebServerRoutes(); 

    // Monitor pin setup //
    attachInterrupt(digitalPinToInterrupt(encoder1.pinA), handleEncoder1Interrupt, CHANGE); // I need to remove the lambda and include the w/ name of interrupt to ensure we are using the correct one

    attachInterrupt(digitalPinToInterrupt(encoder1.pinB), handleEncoder1Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder2.pinA), handleEncoder2Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder2.pinB), handleEncoder2Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder3.pinA), handleEncoder3Interrupt, CHANGE);

    attachInterrupt(digitalPinToInterrupt(encoder3.pinB), handleEncoder3Interrupt, CHANGE);

    // Dim LEDs
    FastLED.setBrightness(24);

    xTaskCreate(
        TaskUpdateDisplay, // Task function
        "DisplayTask",     // Name of the task
        10000,             // Stack size of task
        NULL,              // Parameter of the task
        1,                 // Priority of the task
        NULL);             // Task handle

}

void loop() {rollingPurpleLEDs();} 

void TaskUpdateDisplay(void *pvParameters)
{
    for (;;)
    {
        handleMenuNavigation();
        updateDisplayContent();
    }
}
