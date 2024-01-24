#include <WiFi.h>
#include <HTTPClient.h>

const char *ssid = "ADD_YOUR_WIFI";
const char *password = "ADD_YOUR_PASSWORD";

void setup()
{
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, password);

  Serial.print("Connecting to ");
  Serial.print(ssid);
  Serial.println(" ...");

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(++i);
    Serial.print(' ');
  }

  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
}

void loop()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;
    http.begin("http://192.168.1.17:5000/status"); // Use the IP address of the Flask server
    int httpCode = http.GET();
    // Send the request

    if (httpCode > 0)
    {                                    // Check the returning code
      String payload = http.getString(); // Get the request response payload
      Serial.println(payload);
    }

    http.end(); // Close connection
  }
  delay(10000); // Wait for 10 seconds
}
