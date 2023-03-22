#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Ping.h>
#include <ESPAsyncWebServer.h>
#include <time.h>
#include "constants.h"
#include "utils.h"
#include "wifi_data.h"
#include "wifi_handle_connection.h"
#include "mqtt_handle_connection.h"
#include "coap_handle_connection.h"
#include "http_handle_connection.h"

// Global variables
IPAddress localIp;

const char *ntpServer = "it.pool.ntp.org";
const long gmtOffset_sec = 0;	  /*ECT OFFSET +1 HOURS(3600 SEC)*/
const int daylightOffset_sec = 0; /*1 hour daylight offset*/


String timestamp_received_s = "";
String timestamp_received_us = "";

// Prototype
void printLocalTime();


void setup()
{
	Serial.begin(115200);
	
	// WiFi Connection
	connectToWiFi(ssid, password);
	setLocalIp(localIp);
	Serial.print("Local IP: ");
	Serial.println(localIp);

	// MDNS Connection
	setupMDNS();

	// MQTT Connection
	connectToMQTT(mqttServer, mqttPort, mqttUser, mqttPassword, mqttClient);
	subscribeToTopics();

	// COAP Connection
	coap_set_endpoints();
	coap.start();

	// NTP Connection
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();

	// HTTP Connection
	setup_http_server();
	server.begin();
}

void loop()
{
	if (millis() - last_mqtt_message_received > 60000)
	{
		Serial.println("No message received in 60 seconds");
		ESP.restart();
	}

	checkMQTTConnection(mqttClient);

	coap_call_loop();

	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

}

// FUNCTIONS
void printLocalTime()
{
	struct tm timeinfo;
	unsigned long last_sync_time = millis();
	while (true)
	{
		if (millis() - last_sync_time > 60000)
		{
			Serial.println("Time Sync error! Esp32 will restart");
			ESP.restart();
		}

		if (!getLocalTime(&timeinfo))
		{
			Serial.println("Failed to obtain time");
		}
		else
		{
			Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
			return;
		}
	}
}
