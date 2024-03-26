// Setting Encoder attributes //

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

// void updatePulseTimes(Encoder *encoder, unsigned long pulseTime)
// {
//   // Shift previous times down and add the new time at the end
//   for (int i = 0; i < 4; i++)
//   {
//     encoder->pulseTimes[i] = encoder->pulseTimes[i + 1];
//   }
//   encoder->pulseTimes[4] = pulseTime;
// }

void updatePulseTimes(Encoder *encoder, unsigned long currentTime)
{
  unsigned long newPulseTime = currentTime - encoder->lastInterruptTime;
  // Shift previous times down and add the new time at the end
  for (int i = ENCODER_HISTORY_SIZE - 2; i >= 0; i--)
  {
    encoder->pulseTimes[i + 1] = encoder->pulseTimes[i];
  }
  encoder->pulseTimes[0] = newPulseTime;

  // Update lastInterruptTime for the next pulse
  encoder->lastInterruptTime = currentTime;

  // Optional: Calculate speed based on newPulseTime
  // Assuming encoder resolution and physical parameters are factored in elsewhere
  encoder->speed = 1.0 / newPulseTime; // Simplified; consider actual calculation based on your setup
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

// // Functions to update the encoder position //
// void updateEncoder(Encoder *encoder)
// {
//   unsigned long currentTime = micros();
//   if (currentTime - encoder->lastInterruptTime < encoder->debounceDelay * 1000)
//   {
//     return; // Too soon, ignore this movement
//   }

//   int MSB = digitalRead(encoder->pinA);
//   int LSB = digitalRead(encoder->pinB);
//   int encoded = (MSB << 1) | LSB;
//   int sum = (encoder->lastEncoded << 2) | encoded;

//   if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
//     encoder->position--;
//   if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
//     encoder->position++;

//   encoder->lastEncoded = encoded;
//   encoder->lastInterruptTime = currentTime;

//   // Update pulse times and adjust debounceDelay
//   updatePulseTimes(encoder, currentTime - encoder->lastInterruptTime);
//   adjustDebounceDelay(encoder);
// }

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
  updatePulseTimes(encoder, currentTime);

  int movement = (encoder->lastEncoded << 2) | encoded;
  if (movement == 0b1101 || movement == 0b0100 || movement == 0b0010 || movement == 0b1011)
  {
    encoder->direction = -1; // Assume -1 for counterclockwise
    encoder->position--;
  }
  else if (movement == 0b1110 || movement == 0b0111 || movement == 0b0001 || movement == 0b1000)
  {
    encoder->direction = 1; // Assume 1 for clockwise
    encoder->position++;
  }

  if (encoder->speed > SPEED_THRESHOLD)
  {
    encoder->position += (encoder->direction * POSITION_CHANGE_HIGH_SPEED);
  }
  else
  {
    encoder->position += (encoder->direction * POSITION_CHANGE_LOW_SPEED);
  }
}