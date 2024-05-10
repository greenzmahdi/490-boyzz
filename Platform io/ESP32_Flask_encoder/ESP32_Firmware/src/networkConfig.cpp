#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#include "networkConfig.h"
#include "format.h"
#include "encoder.h"
#include "coordinatePlanes.h"
#include "format.h"
#include "htmlContent.h"

const char *h_ssid = "DRO-491";
const char *h_password = "123456789";

AsyncWebServer server(80);

void setupAccessPoint()
{
    // Setting up the ESP32 as an Access Point //
    WiFi.softAP(h_ssid, h_password);
    Serial.println("Access Point Started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void setupWebServerRoutes()
{
    // Setting up Routes
    // Route for root web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });

    server.on("/get-positions", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["positionX_ABS"] = formatPosition(encoder1.position - planes[currentPlaneIndex].last_ABS[0], isInchMode);
    jsonDoc["positionY_ABS"] = formatPosition(encoder2.position - planes[currentPlaneIndex].last_ABS[1], isInchMode);
    jsonDoc["positionZ_ABS"] = formatPosition(encoder3.position - planes[currentPlaneIndex].last_ABS[2], isInchMode);

    jsonDoc["positionX_INC"] = formatPosition(encoder1.position - planes[currentPlaneIndex].last_INC[0], isInchMode);
    jsonDoc["positionY_INC"] = formatPosition(encoder2.position - planes[currentPlaneIndex].last_INC[1], isInchMode);
    jsonDoc["positionZ_INC"] = formatPosition(encoder3.position - planes[currentPlaneIndex].last_INC[2], isInchMode);

    String jsonString;
    serializeJson(jsonDoc, jsonString);
    request->send(200, "application/json", jsonString); });

    server.on("/toggle-measure-mode", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    toggleMeasurementMode();
    request->send(200, "text/plain", isInchMode ? "INCH" : "MM"); });

    server.on("/reset/x", HTTP_GET, [](AsyncWebServerRequest *request)
              {         
            resetEncoderValue(0); // Reset encoder for X-axis

    request->send(200, "text/plain", "X position reset"); });

    server.on("/reset/y", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    resetEncoderValue(1); // Reset encoder for Y-axis
    request->send(200, "text/plain", "Y position reset"); });

    server.on("/reset/z", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    resetEncoderValue(2); // Reset encoder for Z-axis
    request->send(200, "text/plain", "Z position reset"); });

    server.on("/toggle-mode", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  toggleMode();
                  request->send(200, "text/plain", isABSMode ? "ABS" : "INC"); // Send the new mode back to the client
              });

    server.on("/zero-all-axis", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                resetEncoderValue(0);
                resetEncoderValue(1);
                resetEncoderValue(2);
                request->send(200, "text/plain", "All positions reset"); });

    server.on("/get-current-plane", HTTP_GET, [](AsyncWebServerRequest *request)
              {
                  request->send(200, "text/plain", String(currentPlaneIndex + 1)); // +1 to make it human-readable (1-based index)
              });

    // Endpoint to add a point to the current plane
    server.on("/add-point", HTTP_POST, [](AsyncWebServerRequest *request)
              {
    int x = 0, y = 0, z = 0;
    if (request->hasParam("x") && request->hasParam("y") && request->hasParam("z")) {
        x = request->getParam("x")->value().toInt();
        y = request->getParam("y")->value().toInt();
        z = request->getParam("z")->value().toInt();
        planes[currentPlaneIndex].shapePoints.emplace_back(x, y, z);
        request->send(200, "text/plain", "Point added");
    } else {
        request->send(400, "text/plain", "Missing parameters");
    } });

    server.on("/define-factor", HTTP_POST, [](AsyncWebServerRequest *request)
              {
  if(request->hasParam("factor_mm", true) && request->hasParam("factor_inch", true)){
    float new_factor_mm = request->getParam("factor_mm", true) ->value().toFloat();
    float new_factor_inch = request->getParam("factor_inch", true) ->value().toFloat();
  factor_mm = new_factor_mm;
  factor_inch = new_factor_inch;
  request->send(200, "text/plain", "Factor updated succesfully");
  } else {
      request->send(400, "text/plain", "Missing parameter");
  } });

    // Endpoint to get the last saved point on the current plane
    server.on("/get-last-point", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    if (!planes[currentPlaneIndex].shapePoints.empty()) {
        Point lastPoint = planes[currentPlaneIndex].shapePoints.back();
        char buffer[100];
        snprintf(buffer, sizeof(buffer), "%d,%d,%d", lastPoint.x, lastPoint.y, lastPoint.z);
        request->send(200, "text/plain", buffer);
    } else {
        request->send(404, "text/plain", "No points saved");
    } });

    // Endpoint to get all saved points on the current plane
    server.on("/get-all-points", HTTP_GET, [](AsyncWebServerRequest *request)
              {
      if (!planes[currentPlaneIndex].shapePoints.empty()) {
          String jsonOutput;
          StaticJsonDocument<1024> doc;  // Adjust size as needed based on expected data volume
          JsonArray pointsArray = doc.createNestedArray("points");

          for (const auto& point : planes[currentPlaneIndex].shapePoints) {
              JsonObject pointObj = pointsArray.createNestedObject();
              pointObj["x"] = point.x;
              pointObj["y"] = point.y;
              pointObj["z"] = point.z;
          }

          serializeJson(doc, jsonOutput);
          request->send(200, "application/json", jsonOutput);
      } else {
          request->send(404, "text/plain", "No points saved");
      } });

    server.on("/save-current-position", HTTP_GET, [](AsyncWebServerRequest *request)
              {
    addCurrentPositionToPoint();  // Call the function to add the point
    request->send(200, "text/plain", "Current position saved"); });

    server.begin();
}
