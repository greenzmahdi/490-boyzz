/*|----------------------------------------------------------|*/
/*|Connection sketch to eduroam network (WPA/WPA2) Enteprise |*/
/*|Suitable for almost any ESP32 microcontroller with WiFi   |*/
/*|Raspberry or Arduino WiFi CAN'T USE THIS LIBRARY!!!       |*/
/*|Edited by: Martin Chlebovec (martinius96)                 |*/
/*|Compilation under 2.0.3 Arduino Core and higher worked    |*/
/*|Compilation can be done only using STABLE releases        |*/
/*|Dev releases WILL NOT WORK. (Check your Ard. Core .json)  |*/
/*|WiFi.begin() have more parameters for PEAP connection     |*/
/*|----------------------------------------------------------|*/

//WITHOUT certificate option connection is WORKING (if there is exception set on RADIUS server that will let connect devices without certificate)
//It is DEPRECATED function and standardly turned off, so it must be turned on by your eduroam management at university / organisation

//Connection with certificate WASN'T CONFIRMED ever, so probably that option is NOT WORKING

#include <WiFi.h>      //Wifi library
#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
#include <FastLED.h>
#include <Wire.h>

//Identity for user with password related to his realm (organization)
//Available option of anonymous identity for federation of RADIUS servers or 1st Domain RADIUS servers
#define EAP_ANONYMOUS_IDENTITY "email"  //anonymous@example.com, or you can use also nickname@example.com
#define EAP_IDENTITY "email"            //nickname@example.com, at some organizations should work nickname only without realm, but it is not recommended
#define EAP_PASSWORD "password"                  //password for eduroam account
#define EAP_USERNAME "email"            // the Username is the same as the Identity in most eduroam networks.

// Set web server port number to 80
WiFiServer server(80);

//SSID NAME
const char* ssid = "eduroam";  // eduroam SSID

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



// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0;
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;


//Intermediate CA cert (GEANT OV RSA CA 4) in .pem format
//Used for WiFi connection as trusted CA that issued certificate for wifi.uvt.tuke.sk - Technical university in Košice (Slovakia)
//THIS CERTIFICATE WILL NOT WORK FOR OTHER UNIVERSITIES AND ORGANISATIONS!

// const static char* test_root_ca PROGMEM = \
//     "-----BEGIN CERTIFICATE-----\n" \
//     "MIIG5TCCBM2gAwIBAgIRANpDvROb0li7TdYcrMTz2+AwDQYJKoZIhvcNAQEMBQAw\n" \
//     "gYgxCzAJBgNVBAYTAlVTMRMwEQYDVQQIEwpOZXcgSmVyc2V5MRQwEgYDVQQHEwtK\n" \
//     "ZXJzZXkgQ2l0eTEeMBwGA1UEChMVVGhlIFVTRVJUUlVTVCBOZXR3b3JrMS4wLAYD\n" \
//     "VQQDEyVVU0VSVHJ1c3QgUlNBIENlcnRpZmljYXRpb24gQXV0aG9yaXR5MB4XDTIw\n" \
//     "MDIxODAwMDAwMFoXDTMzMDUwMTIzNTk1OVowRDELMAkGA1UEBhMCTkwxGTAXBgNV\n" \
//     "BAoTEEdFQU5UIFZlcmVuaWdpbmcxGjAYBgNVBAMTEUdFQU5UIE9WIFJTQSBDQSA0\n" \
//     "MIICIjANBgkqhkiG9w0BAQEFAAOCAg8AMIICCgKCAgEApYhi1aEiPsg9ZKRMAw9Q\n" \
//     "r8Mthsr6R20VSfFeh7TgwtLQi6RSRLOh4or4EMG/1th8lijv7xnBMVZkTysFiPmT\n" \
//     "PiLOfvz+QwO1NwjvgY+Jrs7fSoVA/TQkXzcxu4Tl3WHi+qJmKLJVu/JOuHud6mOp\n" \
//     "LWkIbhODSzOxANJ24IGPx9h4OXDyy6/342eE6UPXCtJ8AzeumTG6Dfv5KVx24lCF\n" \
//     "TGUzHUB+j+g0lSKg/Sf1OzgCajJV9enmZ/84ydh48wPp6vbWf1H0O3Rd3LhpMSVn\n" \
//     "TqFTLKZSbQeLcx/l9DOKZfBCC9ghWxsgTqW9gQ7v3T3aIfSaVC9rnwVxO0VjmDdP\n" \
//     "FNbdoxnh0zYwf45nV1QQgpRwZJ93yWedhp4ch1a6Ajwqs+wv4mZzmBSjovtV0mKw\n" \
//     "d+CQbSToalEUP4QeJq4Udz5WNmNMI4OYP6cgrnlJ50aa0DZPlJqrKQPGL69KQQz1\n" \
//     "2WgxvhCuVU70y6ZWAPopBa1ykbsttpLxADZre5cH573lIuLHdjx7NjpYIXRx2+QJ\n" \
//     "URnX2qx37eZIxYXz8ggM+wXH6RDbU3V2o5DP67hXPHSAbA+p0orjAocpk2osxHKo\n" \
//     "NSE3LCjNx8WVdxnXvuQ28tKdaK69knfm3bB7xpdfsNNTPH9ElcjscWZxpeZ5Iij8\n" \
//     "lyrCG1z0vSWtSBsgSnUyG/sCAwEAAaOCAYswggGHMB8GA1UdIwQYMBaAFFN5v1qq\n" \
//     "K0rPVIDh2JvAnfKyA2bLMB0GA1UdDgQWBBRvHTVJEGwy+lmgnryK6B+VvnF6DDAO\n" \
//     "BgNVHQ8BAf8EBAMCAYYwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHSUEFjAUBggr\n" \
//     "BgEFBQcDAQYIKwYBBQUHAwIwOAYDVR0gBDEwLzAtBgRVHSAAMCUwIwYIKwYBBQUH\n" \
//     "AgEWF2h0dHBzOi8vc2VjdGlnby5jb20vQ1BTMFAGA1UdHwRJMEcwRaBDoEGGP2h0\n" \
//     "dHA6Ly9jcmwudXNlcnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FDZXJ0aWZpY2F0aW9u\n" \
//     "QXV0aG9yaXR5LmNybDB2BggrBgEFBQcBAQRqMGgwPwYIKwYBBQUHMAKGM2h0dHA6\n" \
//     "Ly9jcnQudXNlcnRydXN0LmNvbS9VU0VSVHJ1c3RSU0FBZGRUcnVzdENBLmNydDAl\n" \
//     "BggrBgEFBQcwAYYZaHR0cDovL29jc3AudXNlcnRydXN0LmNvbTANBgkqhkiG9w0B\n" \
//     "AQwFAAOCAgEAUtlC3e0xj/1BMfPhdQhUXeLjb0xp8UE28kzWE5xDzGKbfGgnrT2R\n" \
//     "lw5gLIx+/cNVrad//+MrpTppMlxq59AsXYZW3xRasrvkjGfNR3vt/1RAl8iI31lG\n" \
//     "hIg6dfIX5N4esLkrQeN8HiyHKH6khm4966IkVVtnxz5CgUPqEYn4eQ+4eeESrWBh\n" \
//     "AqXaiv7HRvpsdwLYekAhnrlGpioZ/CJIT2PTTxf+GHM6cuUnNqdUzfvrQgA8kt1/\n" \
//     "ASXx2od/M+c8nlJqrGz29lrJveJOSEMX0c/ts02WhsfMhkYa6XujUZLmvR1Eq08r\n" \
//     "48/EZ4l+t5L4wt0DV8VaPbsEBF1EOFpz/YS2H6mSwcFaNJbnYqqJHIvm3PLJHkFm\n" \
//     "EoLXRVrQXdCT+3wgBfgU6heCV5CYBz/YkrdWES7tiiT8sVUDqXmVlTsbiRNiyLs2\n" \
//     "bmEWWFUl76jViIJog5fongEqN3jLIGTG/mXrJT1UyymIcobnIGrbwwRVz/mpFQo0\n" \
//     "vBYIi1k2ThVh0Dx88BbF9YiP84dd8Fkn5wbE6FxXYJ287qfRTgmhePecPc73Yrzt\n" \
//     "apdRcsKVGkOpaTIJP/l+lAHRLZxk/dUtyN95G++bOSQqnOCpVPabUGl2E/OEyFrp\n" \
//     "Ipwgu2L/WJclvd6g+ZA/iWkLSMcpnFb+uX6QBqvD6+RNxul1FaB5iHY=\n" \
//     "-----END CERTIFICATE-----\n";

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


  delay(10);
  Serial.print(F("Connecting to network: "));
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect from WiFi to set new WiFi connection
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_ANONYMOUS_IDENTITY, EAP_IDENTITY, EAP_PASSWORD, test_root_ca); //with CERTIFICATE
  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);  // without CERTIFICATE, RADIUS server EXCEPTION "for old devices" required

  // Example: a cert-file WPA2 Enterprise with PEAP - client certificate and client key required
  //WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD, test_root_ca, client_cert, client_key);

  // Example: TLS with cert-files and no password - client certificate and client key required
  //WiFi.begin(ssid, WPA2_AUTH_TLS, EAP_IDENTITY, NULL, NULL, test_root_ca, client_cert, client_key);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println("");
  Serial.println(F("WiFi is connected!"));
  Serial.println(F("IP address set: "));
  Serial.println(WiFi.localIP());  //print LAN IP
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
          currentLine += c;      // add it to the end of the currentLisne
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
  // yield();
}