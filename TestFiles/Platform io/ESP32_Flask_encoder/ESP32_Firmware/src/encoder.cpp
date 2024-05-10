// Setting Encoder attributes //

#include "encoder.h"
#include "pin.h"
#include "coordinatePlanes.h"

// Initializing encoders attributes and setting their start (refer to encoder struct to see all parameters)
Encoder encoder1 = {PIN_A1, PIN_B1, 0, 0, 0, 0, 0, {0}, 1};
Encoder encoder2 = {PIN_A2, PIN_B2, 0, 0, 0, 0, 0, {0}, 1};
Encoder encoder3 = {PIN_A3, PIN_B3, 0, 0, 0, 0, 0, {0}, 1};


// Constants for the encoder
const int effectivePPR = 24 * 4; // 24 pulses per detent, quadrature mode effectively doubles this
float degreesPerPulse = 360.0 / effectivePPR;

// Function to convert pulses to degrees
float pulsesToDegrees(long pulses) {
  return pulses * degreesPerPulse;
}

const float distancePerPulseInches = 0.515; // Pre-calculated distance per pulse in inches

// Function to convert pulses to distance in inches
float pulsesToDistanceInches(long pulses) {
  return pulses * distancePerPulseInches;
}

void updateEncoderPosition(Encoder *encoder, int newState) {
  int sum = (encoder->lastEncoded << 2) | newState;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoder->position--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoder->position++;
  encoder->lastEncoded = newState;
}

// ISR to handle encoder changes, optimized for minimal processing
void IRAM_ATTR handleEncoderInterrupt(Encoder *encoder) {
  unsigned long currentTime = micros();
  if (currentTime - encoder->lastInterruptTime > encoder->debounceDelay) {
    int newState = (digitalRead(encoder->pinA) << 1) | digitalRead(encoder->pinB);
    updateEncoderPosition(encoder, newState);
    encoder->lastInterruptTime = currentTime;
  }
}

void IRAM_ATTR handleEncoder1Interrupt()
{
    handleEncoderInterrupt(&encoder1); // Assume encoder1 is an instance of Encoder
                                       // for all axis on ABS Mode
    // handleEncoderInterrupt(&encoder1); // Update encoder state

    planes[currentPlaneIndex].encoderValueABS[0] = encoder1.position - planes[currentPlaneIndex].last_ABS[0];
    planes[currentPlaneIndex].encoderValueINC[0] = encoder1.position - planes[currentPlaneIndex].last_INC[0];
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder2Interrupt()
{
    handleEncoderInterrupt(&encoder2); // Assume encoder1 is an instance of Encoder
    planes[currentPlaneIndex].encoderValueABS[1] = encoder2.position - planes[currentPlaneIndex].last_ABS[1];
    planes[currentPlaneIndex].encoderValueINC[1] = encoder2.position - planes[currentPlaneIndex].last_INC[1];
}

// Separate ISRs for each encoder
void IRAM_ATTR handleEncoder3Interrupt()
{
    handleEncoderInterrupt(&encoder3); // Update encoder state
    planes[currentPlaneIndex].encoderValueABS[2] = encoder3.position - planes[currentPlaneIndex].last_ABS[2];
    planes[currentPlaneIndex].encoderValueINC[2] = encoder3.position - planes[currentPlaneIndex].last_INC[2];
}