// Setting Encoder attributes 


#include "encoder.h"

// Fuctions that help find SD to adjust debouncing // 
unsigned long average(unsigned long arr[], int numElements)
{
  unsigned long sum = 0;
  for (int i = 0; i < numElements; i++)
  {
    sum += arr[i];
  }
  return sum / numElements;
}

unsigned long standardDeviation(unsigned long arr[], int numElements, unsigned long mean)
{
  unsigned long variance = 0;
  for (int i = 0; i < numElements; i++)
  {
    variance += (arr[i] - mean) * (arr[i] - mean);
  }
  variance /= numElements;
  return sqrt(variance);
}

void updatePulseTimes(Encoder *encoder, unsigned long pulseTime)
{
  // Shift previous times down and add the new time at the end
  for (int i = 0; i < 4; i++)
  {
    encoder->pulseTimes[i] = encoder->pulseTimes[i + 1];
  }
  encoder->pulseTimes[4] = pulseTime;
}

void adjustDebounceDelay(Encoder *encoder)
{
  // Calculate the average and standard deviation of the encoder's pulse times
  unsigned long avg = average(encoder->pulseTimes, 5);
  unsigned long stdDev = standardDeviation(encoder->pulseTimes, 5, avg);

  // Adjust encoder->debounceDelay based on the speed (average pulse time)
  encoder->debounceDelay = map(avg, 0, 1000, 0, 5);

  // Further adjust encoder->debounceDelay based on consistency (standard deviation)
  encoder->debounceDelay += map(stdDev, 0, 500, 0, 5);

  // Ensure encoder->debounceDelay stays within the desired range of 0-5
  encoder->debounceDelay = constrain(encoder->debounceDelay, 0, 5);
}

// Functions to update the encoder position // 
void updateEncoder(Encoder *encoder)
{
  unsigned long currentTime = micros();
  if (currentTime - encoder->lastInterruptTime < encoder->debounceDelay * 1000)
  {
    return; // Too soon, ignore this movement
  }

  int MSB = digitalRead(encoder->pinA);
  int LSB = digitalRead(encoder->pinB);
  int encoded = (MSB << 1) | LSB;
  int sum = (encoder->lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoder->position--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoder->position++;

  encoder->lastEncoded = encoded;
  encoder->lastInterruptTime = currentTime;

  // Update pulse times and adjust debounceDelay
  updatePulseTimes(encoder, currentTime - encoder->lastInterruptTime);
  adjustDebounceDelay(encoder);
}