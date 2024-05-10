volatile int encoderPos = 0;
volatile int lastEncoded = 0;
volatile unsigned long lastInterruptTime = 0;
unsigned long timeBetweenPulses = 0;
unsigned long pulseTimes[5]; // Array to store the last 5 pulse times for consistency calculation
const int pinA = 33;
const int pinB = 26;
unsigned long debounceDelay = 1;

void setup() {
  Serial.begin(115200);
  pinMode(pinA, INPUT_PULLUP);
  pinMode(pinB, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(pinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(pinB), updateEncoder, CHANGE);
  memset(pulseTimes, 0, sizeof(pulseTimes)); // Initialize pulseTimes array
}

void loop() {
  static int lastPos = 0;
  if (encoderPos != lastPos) {
    Serial.println(encoderPos);
    lastPos = encoderPos;
  }
  // Dynamically adjust debounceDelay based on speed and signal consistency
  adjustDebounceDelay();
}

void updateEncoder() {
  unsigned long currentTime = micros();
  if (currentTime - lastInterruptTime < debounceDelay * 1000) {
    return; // Too soon, ignore this movement
  }

  int MSB = digitalRead(pinA);
  int LSB = digitalRead(pinB);
  int encoded = (MSB << 1) | LSB;
  int sum = (encoder->lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos++;

  lastEncoded = encoded;
  lastInterruptTime = currentTime;
  // Update pulse times for consistency calculation
  updatePulseTimes(currentTime - lastInterruptTime);
}

void updatePulseTimes(unsigned long pulseTime) {
  // Shift previous times down and add the new time at the end
  for (int i = 0; i < 4; i++) {
    pulseTimes[i] = pulseTimes[i + 1];
  }
  pulseTimes[4] = pulseTime;
}

// void adjustDebounceDelay() {
//   unsigned long avg = average(pulseTimes, 5);
//   unsigned long stdDev = standardDeviation(pulseTimes, 5, avg);
//   // Here, we adjust debounceDelay based on the average pulse time (speed) and its standard deviation (consistency)
//   // This is a simplified example; you'll need to tailor these calculations to your specific application
//   debounceDelay = map(avg, 0, 1000, 0, 5); // Adjust based on speed (original 110)
//   debounceDelay += map(stdDev, 0, 500, 0, 5); // Adjust based on consistency
//   debounceDelay = constrain(debounceDelay, 0, 5); // Ensure debounceDelay stays within a reasonable range
// }

void adjustDebounceDelay() {
  // Assuming pulseTimes is an array of recent pulse times
  unsigned long avg = average(pulseTimes, 5); // Calculate the average of the last 5 pulse times
  unsigned long stdDev = standardDeviation(pulseTimes, 5, avg); // Calculate the standard deviation

  // Adjust debounceDelay based on the speed (average pulse time)
  // The mapping here is purely hypothetical and should be adjusted based on empirical observations
  // For example, if avg is between 0 and 1000 microseconds, map it to a range of 0-5
  debounceDelay = map(avg, 0, 1000, 0, 5);

  // Further adjust debounceDelay based on consistency (standard deviation of pulse times)
  // The idea is to increase debounceDelay for more variability to reduce noise
  // Again, adjust the range based on your observations
  debounceDelay += map(stdDev, 0, 500, 0, 5); // Adjust based on consistency

  // Ensure debounceDelay stays within the desired range of 0-5
  debounceDelay = constrain(debounceDelay, 0, 5);
}


unsigned long average(unsigned long arr[], int numElements) {
  unsigned long sum = 0;
  for (int i = 0; i < numElements; i++) {
    sum += arr[i];
  }
  return sum / numElements;
}

unsigned long standardDeviation(unsigned long arr[], int numElements, unsigned long mean) {
  unsigned long variance = 0;
  for (int i = 0; i < numElements; i++) {
    variance += (arr[i] - mean) * (arr[i] - mean);
  }
  variance /= numElements;
  return sqrt(variance);
}

