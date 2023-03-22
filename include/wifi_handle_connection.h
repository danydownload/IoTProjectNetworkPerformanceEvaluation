#ifndef WIFI_HANDLE_CONNECTION_H
#define WIFI_HANDLE_CONNECTION_H

#include <WiFi.h>

void connectToWiFi(const char* ssid, const char* password);

void setLocalIp(IPAddress &localIp);

#endif