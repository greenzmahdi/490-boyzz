#ifndef ENCODER_SETUP_H
#define ENCODER_SETUP_H
#include <WiFi.h>
#include <AccelStepper.h>

#define ENCODER_HISTORY_SIZE 4
#define SPEED_THRESHOLD 0.01 // Example threshold, adjust based on your application
#define POSITION_CHANGE_HIGH_SPEED 2 // needs to be tuned
#define POSITION_CHANGE_LOW_SPEED 1 // needs to be tuned 

  struct Encoder
{
  const uint8_t pinA;
  const uint8_t pinB;
  volatile int position;
  volatile int positionInc;
  volatile int positionABS;
  volatile int lastEncoded;
  volatile unsigned long lastInterruptTime;
  unsigned long pulseTimes[ENCODER_HISTORY_SIZE]; 
  unsigned long debounceDelay;
  float speed;
  int direction;
};

unsigned long average(unsigned long arr[], int numElements);
unsigned long standardDeviation(unsigned long arr[], int numElements, unsigned long mean);
void updatePulseTimes(Encoder *encoder, unsigned long pulseTime);
void adjustDebounceDelay(Encoder *encoder);
void updateEncoder(Encoder *encoder);
void updateEncoderPosition(Encoder *encoder, int newState);
// ISR to handle encoder changes, optimized for minimal processing
void IRAM_ATTR handleEncoderInterrupt(Encoder *encoder);
// Function to convert pulses to degrees
float pulsesToDegrees(long pulses);
float pulsesToDistanceInches(long pulses);


#endif