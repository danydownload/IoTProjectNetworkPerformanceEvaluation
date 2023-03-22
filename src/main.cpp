#include <WiFi.h>
#include <Arduino.h>
#include <time.h>
#include "messages.h"
#include "utils.h"
#include "wifi_data.h"
#include "mqtt_handle_connection.h"
#include "wifi_handle_connection.h"


BluetoothSerial SerialBT;
WiFiClient wifiClient;


// #define DEVICE_ADDRESS "B0:3C:DC:D0:5B:2D"
const char *bluetooth_server = "ESP32 Server";
const char *mqtt_server = "MSI.local";

const char *mqttServer = "MSI.local";
const int mqttPort = 1883;
PubSubClient mqttClient = PubSubClient(mqttServer, mqttPort, wifiClient);

const char *ntpServer = "it.pool.ntp.org";
const long gmtOffset_sec = 0;	  /*ECT OFFSET +1 HOURS(3600 SEC)*/
const int daylightOffset_sec = 0; /*1 hour daylight offset*/

const char *mqtt_nodered_throughput_topic = "nodered/bt/mqtt/throughput";
const char *coap_nodered_throughput_topic = "nodered/bt/coap/throughput";
const char *http_nodered_throughput_topic = "nodered/bt/http/throughput";

const char *mqtt_nodered_packet_loss_topic = "nodered/bt/mqtt/packet_loss";
const char *coap_nodered_packet_loss_topic = "nodered/bt/coap/packet_loss";
const char *http_nodered_packet_loss_topic = "nodered/bt/http/packet_loss";

void printLocalTime();

void setup()
{
	Serial.begin(115200);
	SerialBT.begin("ESP32 Client", 1); // master mode

	// Connect to WiFi
	connectToWiFi(ssid, password);

	// NTP Connection
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();

	// MQTT Connection
	connectToMQTT(mqttServer, mqttPort);
}

void loop()
{
	checkMQTTConnection();

	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	if (!SerialBT.connect("ESP32 Server"))
	{
		Serial.println("Failed to connect to server");
		delay(1000);
	}
	else
	{
		Serial.println("Connected to server");

		compute_packet_loss_and_latency_and_throughput("MQTT", "", "hello", "");
		compute_packet_loss_and_latency_and_throughput("COAP", "hello", "", "");
		compute_packet_loss_and_latency_and_throughput("HTTP", "http://MSI.local:8080", "", "PUT");
		
		delay(1000);
	}
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
