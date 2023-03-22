#ifndef WIFI_HANDLE_CONNECTION_H
#define WIFI_HANDLE_CONNECTION_H

#include <WiFi.h>

extern WiFiClient wifiClient;

void connectToWiFi(const char* ssid, const char* password);

void setLocalIp(IPAddress &localIp);

void setupMDNS();






#endif