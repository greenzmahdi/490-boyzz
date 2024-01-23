#include <Wire.h>
#include <FastLED.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

/// GLOBAL VARIABLES ////

const int PIN_I2C_SCL = 16;
const int PIN_I2C_SDA = 13;

const int PIN_LED = 12;

const int PIN_A1 = 33;
const int PIN_A2 = 32;
const int PIN_A3 = 35;
const int PIN_A4 = 34;
const int PIN_A5 = 18;
const int PIN_A6 = 27;
const int PIN_B1 = 26;
const int PIN_B2 = 4;
const int PIN_B3 = 17;
const int PIN_B4 = 22;
const int PIN_B5 = 23;
const int PIN_B6 = 14;

const int RefreshDelay = 5;
const int LEDNum = 12;
const int LEDColorDisconnected[] = { 0, 0, 0 };
const int LEDColorConnectedA[] = { 204, 102, 0 };  // change the color of the LED light A
const int LEDColorConnectedB[] = { 0, 204, 0 };    // change the color of the LED light B
const char* MenuItems[] = { "Connection 1 (X)", "Connection 2 (Y)", "Connection 3", "Connection 4", "Connection 5", "Connection 6" };

const int IdxZ1 = 5;
const int IdxZ2 = 4;
const int IdxZ3 = 3;
const int IdxZ4 = 2;
const int IdxZ5 = 1;
const int IdxZ6 = 0;

// Encoder variables
volatile int encoderPos = 0;  // This variable will increase or decrease based on the encoder's rotation
unsigned long lastEncoderRead = 0;

int lastEncoded = 0;  // This will store the last state of the encoder

// DO NOT EDIT

CRGB LEDs[LEDNum];
Adafruit_SSD1306 LCD(128, 64, &Wire);
bool MotorStatesAPrev[] = { false, false, false, false, false, false };
bool MotorStatesBPrev[] = { false, false, false, false, false, false };
bool MotorStatesZPrev[] = { false, false, false, false, false, false };
bool ButtonStatesPrev[] = { false, false, false, false, false };
int MotorChannelSelected = 0;
int MotorChannelWatched = -1;

//// LED FUNCTIONS ////
void LEDSet(const int idx, const int colorR, const int colorG, const int colorB) {
  if ((idx < 0) && (idx >= LEDNum))
    return;
  if ((colorR < 0) && (colorR >= 256))
    return;
  if ((colorG < 0) && (colorG >= 256))
    return;
  if ((colorB < 0) && (colorB >= 256))
    return;

  LEDs[idx] = CRGB(colorR, colorG, colorB);
}

void LEDSet(const int idx, const int* color) {
  LEDSet(idx, color[0], color[1], color[2]);
}


void LEDShow() {
  FastLED.show();
}

void LEDInit() {
  FastLED.addLeds<WS2812, PIN_LED, GRB>(LEDs, LEDNum);
}

//// LCD FUNCTIONS ////

void LCDRectFill(int x, int y, int w, int h, int color) {
  LCD.fillRect(x, y, w, h, color);
}

void LCDTextDraw(int x, int y, const char* text, byte size, int colorFont, int colorBG) {
  LCD.setCursor(x, y);
  LCD.setTextSize(size);
  LCD.setTextColor(colorFont, colorBG);
  LCD.print(text);
  LCD.display();
}

void LCDScreenClear() {
  LCD.clearDisplay();
  LCD.display();
  LCD.setTextColor(WHITE, BLACK);
}

void LCDInit() {
  LCD.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


//// I2C FUNCTIONS ////
byte I2CReadRegs(int address, int size) {
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(address, size);

  return Wire.read();
}

bool I2CReadReg(int address, int size, int idx) {
  byte regs = I2CReadRegs(address, size);

  return bitRead(regs, idx);
}



//// BUTTON FUNCTIONS ////

bool ButtonRead(int idx) {
  // 0 - left
  // 1 - center
  // 2 - up
  // 3 - down
  // 4 - right

  if ((idx < 0) || (idx > 4))
    return false;

  return !I2CReadReg(0x20, 1, idx);
}

bool ButtonLeftPressed() {
  return ButtonRead(0);
}

bool ButtonCenterPressed() {
  return ButtonRead(1);
}

bool ButtonUpPressed() {
  return ButtonRead(2);
}

bool ButtonDownPressed() {
  return ButtonRead(3);
}

bool ButtonRightPressed() {
  return ButtonRead(4);
}




void encoderISR() {
  unsigned long currentTime = millis();
  if (currentTime - lastEncoderRead < 6) {  // original: 5 milliseconds debounce time
    return;
  }
  lastEncoderRead = currentTime;

  int newA = digitalRead(PIN_A1);
  int newB = digitalRead(PIN_B1);

  int encoded = (newA << 1) | newB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos--;

  lastEncoded = encoded;
}




void setup() {
  Serial.begin(9600);

  pinMode(PIN_I2C_SCL, OUTPUT);
  pinMode(PIN_I2C_SDA, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_A1, INPUT);
  pinMode(PIN_A2, INPUT);
  pinMode(PIN_A3, INPUT);
  pinMode(PIN_A4, INPUT);
  pinMode(PIN_A5, INPUT);
  pinMode(PIN_A6, INPUT);
  pinMode(PIN_B1, INPUT);
  pinMode(PIN_B2, INPUT);
  pinMode(PIN_B3, INPUT);
  pinMode(PIN_B4, INPUT);
  pinMode(PIN_B5, INPUT);
  pinMode(PIN_B6, INPUT);

  Wire.setPins(PIN_I2C_SDA, PIN_I2C_SCL);
  Wire.begin();
  Wire.setClock(400000);
  // Set the brightness (0-255)
  FastLED.setBrightness(24);

  LCDInit();
  LCDScreenClear();

  LEDInit();


  attachInterrupt(digitalPinToInterrupt(PIN_A1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B1), encoderISR, CHANGE);


  for (int i = 0; i < LEDNum; i++)
    LEDSet(i, LEDColorConnectedA);

  LEDShow();

  LCDTextDraw(10, 6, "TTT Platform.IO", 1, 1, 0);
  delay(1000);
  // delay(2000);
  // MenuPrint();
}

void loop() {
  bool stateA1 = digitalRead(PIN_A1);
  bool stateA2 = digitalRead(PIN_A2);
  bool stateA3 = digitalRead(PIN_A3);
  bool stateA4 = digitalRead(PIN_A4);
  bool stateA5 = digitalRead(PIN_A5);
  bool stateA6 = digitalRead(PIN_A6);
  bool stateB1 = digitalRead(PIN_B1);
  bool stateB2 = digitalRead(PIN_B2);
  bool stateB3 = digitalRead(PIN_B3);
  bool stateB4 = digitalRead(PIN_B4);
  bool stateB5 = digitalRead(PIN_B5);
  bool stateB6 = digitalRead(PIN_B6);
  
  bool stateZ1 = !I2CReadReg(0x20, 2, 8 + IdxZ1);
  bool stateZ2 = !I2CReadReg(0x20, 2, 8 + IdxZ2);
  bool stateZ3 = !I2CReadReg(0x20, 2, 8 + IdxZ3);
  bool stateZ4 = !I2CReadReg(0x20, 2, 8 + IdxZ4);
  bool stateZ5 = !I2CReadReg(0x20, 2, 8 + IdxZ5);
  bool stateZ6 = !I2CReadReg(0x20, 2, 8 + IdxZ6);

  bool stateButtonCenter = ButtonCenterPressed();
  bool stateButtonUp = ButtonUpPressed();
  bool stateButtonDown = ButtonDownPressed();
  bool stateButtonLeft = ButtonLeftPressed();
  bool stateButtonRight = ButtonRightPressed();

  {
    stateA1 ? LEDSet(1, LEDColorDisconnected) : LEDSet(1, LEDColorConnectedA);
    stateB1 ? LEDSet(0, LEDColorDisconnected) : LEDSet(0, LEDColorConnectedB);

    stateA2 ? LEDSet(3, LEDColorDisconnected) : LEDSet(3, LEDColorConnectedA);
    stateB2 ? LEDSet(2, LEDColorDisconnected) : LEDSet(2, LEDColorConnectedB);

    stateA3 ? LEDSet(5, LEDColorDisconnected) : LEDSet(5, LEDColorConnectedA);
    stateB3 ? LEDSet(4, LEDColorDisconnected) : LEDSet(4, LEDColorConnectedB);

    stateA4 ? LEDSet(7, LEDColorDisconnected) : LEDSet(7, LEDColorConnectedA);
    stateB4 ? LEDSet(6, LEDColorDisconnected) : LEDSet(6, LEDColorConnectedB);

    stateA5 ? LEDSet(9, LEDColorDisconnected) : LEDSet(9, LEDColorConnectedA);
    stateB5 ? LEDSet(8, LEDColorDisconnected) : LEDSet(8, LEDColorConnectedB);

    stateA6 ? LEDSet(11, LEDColorDisconnected) : LEDSet(11, LEDColorConnectedA);
    stateB6 ? LEDSet(10, LEDColorDisconnected) : LEDSet(10, LEDColorConnectedB);

    LEDShow();
  }

  // Update the encoder position
  // updateEncoder();

  MotorStatesAPrev[0] = stateA1;
  MotorStatesAPrev[1] = stateA2;
  MotorStatesAPrev[2] = stateA3;
  MotorStatesAPrev[3] = stateA4;
  MotorStatesAPrev[4] = stateA5;
  MotorStatesAPrev[5] = stateA6;

  MotorStatesBPrev[0] = stateB1;
  MotorStatesBPrev[1] = stateB2;
  MotorStatesBPrev[2] = stateB3;
  MotorStatesBPrev[3] = stateB4;
  MotorStatesBPrev[4] = stateB5; 
  MotorStatesBPrev[5] = stateB6;

  MotorStatesZPrev[0] = stateZ1;
  MotorStatesZPrev[1] = stateZ2;
  MotorStatesZPrev[2] = stateZ3;
  MotorStatesZPrev[3] = stateZ4;
  MotorStatesZPrev[4] = stateZ5;
  MotorStatesZPrev[5] = stateZ6;
  
  ButtonStatesPrev[0] = stateButtonCenter;
  ButtonStatesPrev[1] = stateButtonUp;
  ButtonStatesPrev[2] = stateButtonDown;
  ButtonStatesPrev[3] = stateButtonLeft;
  ButtonStatesPrev[4] = stateButtonRight;

LCDRectFill(70, 54, 20, 8, BLACK);  // Adjust the x, y, width, and height as needed

  // Display the encoder position on the LCD
  char buffer[10];
  sprintf(buffer, "%d", encoderPos);
  LCDTextDraw(70, 54, buffer, 1, WHITE, BLACK);

  delayMicroseconds(RefreshDelay);
}