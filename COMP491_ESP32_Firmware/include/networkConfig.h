#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H
#include <ESPAsyncWebServer.h>

extern const char *h_ssid;
extern const char *h_password;

extern AsyncWebServer server;
// extern AsyncWebServer server(80);

void setupAccessPoint();
void setupWebServerRoutes();


#endif