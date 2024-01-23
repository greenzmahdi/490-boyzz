// #include <Arduino.h>
// #include <WiFi.h>
// #include <HTTPClient.h>

// // Replace with your network credentials
// const char* ssid = "[SOSA_HOME]";
// const char* password = "armando1!";

// // Flask server URL for getting LED state
// const char* serverUrl = "http://192.168.1.17:5000/"; // Replace with your Flask server IP and port

// // LED Pin
// const int LED_PIN = 12;

// void setup() {
//   Serial.begin(115200);
//   pinMode(LED_PIN, OUTPUT);

//   // Connect to Wi-Fi
//   WiFi.begin(ssid, password);
//   while (WiFi.status() != WL_CONNECTED) {
//     delay(1000);
//     Serial.println("Connecting to WiFi...");
//   }
//   Serial.println("Connected to WiFi");
// }

// void loop() {
//   if (WiFi.status() == WL_CONNECTED) {
//     HTTPClient http;
//     http.begin(serverUrl); //Specify request destination
//     int httpCode = http.GET(); //Send the request

//     if (httpCode > 0) { //Check the returning code
//       String payload = http.getString(); //Get the request response payload
//       Serial.println(payload); //Print the response payload

//       if (payload == "on") {
//         digitalWrite(LED_PIN, HIGH); // Turn LED on
//       } else {
//         digitalWrite(LED_PIN, LOW); // Turn LED off
//       }

//     } else {
//       Serial.println("Error on HTTP request");
//     }

//     http.end(); //Free resources
//   } else {
//     Serial.println("WiFi not connected");
//   }

//   delay(10000); // Wait for 10 seconds before next check
// }
