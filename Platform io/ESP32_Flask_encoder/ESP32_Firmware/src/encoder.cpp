// Setting Encoder attributes //

#include "encoder.h"

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