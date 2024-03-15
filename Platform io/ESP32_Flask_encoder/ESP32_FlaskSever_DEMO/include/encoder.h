// #ifndef ENCODER_SETUP_H
// #define ENCODER_SETUP_H
// #include <WiFi.h>
// struct Encoder
// {
//   const uint8_t pinA;
//   const uint8_t pinB;
//   volatile int position;
//   volatile int positionInc;
//   volatile int lastEncoded;
//   volatile unsigned long lastInterruptTime;
//   unsigned long pulseTimes[5]; // Array to store the last 5 pulse times for consistency calculation
//   unsigned long debounceDelay;
// };

// unsigned long average(unsigned long arr[], int numElements);
// unsigned long standardDeviation(unsigned long arr[], int numElements, unsigned long mean);
// void updatePulseTimes(Encoder *encoder, unsigned long pulseTime);
// void adjustDebounceDelay(Encoder *encoder);
// void updateEncoder(Encoder *encoder);
// #endif