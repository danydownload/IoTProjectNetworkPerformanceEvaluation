#include "wifi.h"
#include <ESPmDNS.h>

WiFiClient wifiClient;

void connectToWiFi(const char *ssid, const char *password)
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start_time = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    if ((millis() - start_time) > 5000)
    {
      Serial.println("Failed to connect to WiFi. Restarting...");
      ESP.restart();
    }
  }
  Serial.println("Connected to WiFi");
}

void setLocalIp(IPAddress &localIp)
{
  localIp = WiFi.localIP();
  Serial.print("Local IP: ");
  Serial.println(localIp);
}

void setupMDNS()
{
  if (!MDNS.begin("Esp32-Server"))
  {
    Serial.println("Error setting up mDNS.");
    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS started");
}