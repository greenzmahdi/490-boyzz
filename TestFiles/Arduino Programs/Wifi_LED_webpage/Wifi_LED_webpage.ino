/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>
#include <FastLED.h>
#include <Wire.h>

// extra pw
// gabygon19

// const char* ssid = "gabyshome";
// const char* password = "gabygon19";

// // Replace with your network credentials
const char* ssid = "daPrince";
const char* password = "1234567890";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output12State = "off";
String output27State = "off";
String encoderposition = "0";

// Assign output variables to GPIO pins
const int output26 = 26;
const int output27 = 27;

const int PIN_LED = 12;
const int LEDColorConnected[] = { 204, 102, 0 };  // change the color of the LED light A
const int LEDColorDisconnected[] = { 0, 0, 0 };

const int LEDNum = 12;
CRGB LEDs[LEDNum];

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

/// Encoder variables ///
volatile int encoderPos = 0;  // This variable will increase or decrease based on the encoder's rotation
unsigned long lastEncoderRead = 0;

int lastEncoded = 0;  // This will store the last state of the encoder

//// I2C FUNCTIONS ////

bool I2CReadReg(int address, int size, int idx) {
  byte regs = I2CReadRegs(address, size);

  return bitRead(regs, idx);
}

byte I2CReadRegs(int address, int size) {
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission();

  Wire.requestFrom(address, size);

  return Wire.read();
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

//// LED FUNCTIONS ////

void LEDSet(const int idx, const int* color) {
  LEDSet(idx, color[0], color[1], color[2]);
}

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

void LEDShow() {
  FastLED.show();
}

void LEDInit() {
  FastLED.addLeds<WS2812, PIN_LED, GRB>(LEDs, LEDNum);
}



// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  // Initialize the output variables as outputs
  // pinMode(output26, OUTPUT);
  // pinMode(output27, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  // Set outputs to LOW
  // digitalWrite(output26, LOW);
  // digitalWrite(output27, LOW);
  digitalWrite(PIN_LED, LOW);

  // set up the pins
  pinMode(PIN_A1, INPUT);
  pinMode(PIN_B1, INPUT);

  attachInterrupt(digitalPinToInterrupt(PIN_A1), encoderISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_B1), encoderISR, CHANGE);

  FastLED.setBrightness(24);

  // LEDInit();
  for (int i = 0; i < LEDNum; i++)
    LEDSet(i, LEDColorConnected);

  // LEDShow();

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");                                             // print a message out in the serial port
    String currentLine = "";                                                   // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;
        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off
            if (header.indexOf("GET /12/on") >= 0) {
              Serial.println("GPIO 12 on");
              output12State = "on";
              digitalWrite(output26, HIGH);
              //===============================================
              FastLED.setBrightness(24);

              LEDInit();
              for (int i = 0; i < LEDNum; i++)
                LEDSet(i, LEDColorConnected);

              LEDShow();
              //===============================================
            } else if (header.indexOf("GET /12/off") >= 0) {
              Serial.println("GPIO 12 off");
              output12State = "off";
              digitalWrite(output26, LOW);
              //===============================================
              FastLED.setBrightness(24);

              LEDInit();
              for (int i = 0; i < LEDNum; i++)
                LEDSet(i, LEDColorDisconnected);

              LEDShow();
              //===============================================
            } else if (header.indexOf("GET /27/on") >= 0) {
              Serial.println("GPIO 27 on");
              output27State = "on";
              digitalWrite(output27, HIGH);
            } else if (header.indexOf("GET /27/off") >= 0) {
              Serial.println("GPIO 27 off");
              output27State = "off";
              digitalWrite(output27, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // Bootstrap CSS
            client.println("<link rel=\"stylesheet\" href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/css/bootstrap.min.css\">");
            client.println("<style>");
            client.println("body { font-family: Arial, sans-serif; text-align: center; background-color: #f4f4f4; }");
            client.println(".button { background-color: #007bff; border: none; color: white; padding: 12px 24px; text-decoration: none; font-size: 20px; margin: 10px; cursor: pointer; border-radius: 5px; transition: background-color 0.3s; }");
            client.println(".button:hover { background-color: #0056b3; }");
            client.println(".status { font-size: 24px; margin-top: 20px; }");
            client.println("</style></head>");

            client.println("<body>");
            client.println("<h1>ESP32 Web Server</h1>");

            // Display current state, and ON/OFF buttons for GPIO 26
            client.println("<div class=\"status\">GPIO 12 - State <span id=\"gpio12State\">" + output12State + "</span></div>");
            if (output12State == "off") {
              client.println("<p><a href=\"/12/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/12/off\"><button class=\"button\">OFF</button></a></p>");
            }

            // Display current state, and ON/OFF buttons for GPIO 27
            // Uncomment and modify if needed
            // client.println("<div class=\"status\">GPIO 27 - State <span id=\"gpio27State\">" + output27State + "</span></div>");
            // if (output27State == "off") {
            //     client.println("<p><a href=\"/27/on\"><button class=\"button\">ON</button></a></p>");
            // } else {
            //     client.println("<p><a href=\"/27/off\"><button class=\"button\">OFF</button></a></p>");
            // }

            client.println("<div class=\"status\">Encoder Position: <span id=\"encoderPosition\">" + String(encoderPos) + "</span></div>");
            client.println("<script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\"></script>");
            client.println("<script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.7/umd/popper.min.js\"></script>");
            client.println("<script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.3.1/js/bootstrap.min.js\"></script>");
            // Add JavaScript here to dynamically update encoder position
            client.println("</body></html>");


            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
