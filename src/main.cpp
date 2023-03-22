#include <Arduino.h>
#include "BluetoothSerial.h"
#include "wifi_handle_connection.h"
#include <time.h>
#include "mqtt_handle_connection.h"
#include "wifi_data.h"

BluetoothSerial SerialBT;
WiFiClient wifiClient;

const char *mqttServer = "MSI.local";
const int mqttPort = 1883;
PubSubClient mqttClient = PubSubClient(mqttServer, mqttPort, wifiClient);

const char *ntpServer = "it.pool.ntp.org";
const long gmtOffset_sec = 0;	  /*ECT OFFSET +1 HOURS(3600 SEC)*/
const int daylightOffset_sec = 0; /*1 hour daylight offset*/

const char *mqtt_nodered_latency_topic = "nodered/bt/mqtt/latency";
const char *coap_nodered_latency_topic = "nodered/bt/coap/latency";
const char *http_nodered_latency_topic = "nodered/bt/http/latency";

long long compute_latency(String timestamp_received_s, String timestamp_received_us, struct timeval tv);
void printLocalTime();

unsigned long time_not_connected = 0;

void setup()
{
	Serial.begin(115200);
	SerialBT.begin("ESP32 Server", 0); // slave mode

	// Connect to WiFi
	connectToWiFi(ssid, password);

	// NTP Connection
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();

	// MQTT Connection
	connectToMQTT(mqttServer, mqttPort);
}

int times_no_connected = 0;

void loop()
{

	checkMQTTConnection();

	if (!SerialBT.connected())
	{
		times_no_connected++;
		// Serial.println("The client disconnected or something went wrong");
		// delay(1000);
		// if (times_no_connected == 50)
		if (millis() - time_not_connected > 25000)
		{
			Serial.println("Restarting...");
			ESP.restart();
		}
	}

	if (SerialBT.available())
	{
		times_no_connected = 0;
		// Allocate memory for the message
		// Serial.println("Allocating memory for the message...");
		uint8_t *mqtt_message = NULL;
		// int max_message_size = (message_counter < 10) ? 64 : 1024;
		int max_message_size = 1024;
		mqtt_message = (uint8_t *)malloc(max_message_size * sizeof(uint8_t));

		// int size = SerialBT.readBytesUntil('\n', mqtt_message, max_message_size);

		// Read the message that is sent by the client and indicate the size of the message to be read
		String dimension = SerialBT.readStringUntil('\n');
		int dim = dimension.toInt();
		Serial.println("Dimension: " + String(dim));
		int size = SerialBT.readBytes(mqtt_message, dim);

		struct timeval tv;
		gettimeofday(&tv, NULL);
		unsigned long time_for_reading = millis();

		unsigned long total_time_for_reading = millis() - time_for_reading;
		Serial.println("Time for reading: " + String(total_time_for_reading) + " ms");

		Serial.println("Size: " + String(size) + " bytes");

		// for (int i = 0; i < size; i++)
		// {

		// 	Serial.print(String(i) + ": " + String(char(mqtt_message[i])) + "\n");
		// }

		// if mqtt_message starts with 'M'
		if ((char)mqtt_message[0] == 'M')
		{
			Serial.println("MQTT message received");
			if (size == 0)
			{
				// Serial.println("Error: Message size mismatch");
				free(mqtt_message); // Free the dynamically allocated memory when done
				return;
			}
			else if (size < 100) // size + the first byte indicating the type of the message
			{

				String seconds = "";
				for (int i = 44; i < 54; i++) // index that contains the seconds
				{
					seconds += (char)mqtt_message[i];
				}
				// da 60 a 65 milliseconds
				String milliseconds = "";
				for (int i = 55; i < 61; i++) // index that contains the milliseconds
				{
					milliseconds += (char)mqtt_message[i];
				}

				long long latency_ms = compute_latency(seconds, milliseconds, tv);
				if (latency_ms != -1)
				{
					Serial.println("[MQTT] Latency: " + String(latency_ms) + " ms");
					mqttClient.publish(mqtt_nodered_latency_topic, String(latency_ms).c_str());
				}

				// Send a string to the client containing "OK"
				String ok = "OK";
				SerialBT.print(ok);
			}
			else if (size > 100)
			{
				SerialBT.flush();
				Serial.println("MQTT Throughput message received");
			}

			// Free the dynamically allocated memory when done
			// Serial.println("Freeing memory...");
		}

		if ((char)mqtt_message[0] == 'C')
		{
			Serial.println("COAP message received");
			// da 41 a 50

			// da 52 a 57

			if (size == 0)
			{
				// Serial.println("Error: Message size mismatch");
				free(mqtt_message); // Free the dynamically allocated memory when done
				return;
			}
			else if (size < 100)
			{
				String seconds = "";
				for (int i = 41; i < 51; i++) // index that contains the seconds
				{
					seconds += (char)mqtt_message[i];
				}
				// da 60 a 65 milliseconds
				String milliseconds = "";
				for (int i = 52; i < 58; i++) // index that contains the milliseconds
				{
					milliseconds += (char)mqtt_message[i];
				}

				long long latency_ms = compute_latency(seconds, milliseconds, tv);
				if (latency_ms != -1)
				{
					Serial.println("[COAP] Latency: " + String(latency_ms) + " ms");
					mqttClient.publish(coap_nodered_latency_topic, String(latency_ms).c_str());
				}

				// Send a string to the client containing "OK"
				String ok = "OK";
				SerialBT.print(ok);
			}
			else if (size > 100)
			{
				SerialBT.flush();
				Serial.println("COAP Throughput message received");
			}
		}

		if ((char)mqtt_message[0] == 'H')
		{
			Serial.println("HTTP message received");

			if (size == 0)
			{
				// Serial.println("Error: Message size mismatch");
				free(mqtt_message); // Free the dynamically allocated memory when done
				return;
			}
			else if (size < 200)
			{
				String seconds = "";
				for (int i = 160; i < 170; i++) // index that contains the seconds
				{
					seconds += (char)mqtt_message[i];
				}
				// da 60 a 65 milliseconds
				String milliseconds = "";
				for (int i = 171; i < 177; i++) // index that contains the milliseconds
				{
					milliseconds += (char)mqtt_message[i];
				}

				long long latency_ms = compute_latency(seconds, milliseconds, tv);
				if (latency_ms != -1)
				{
					Serial.println("[HTTP] Latency: " + String(latency_ms) + " ms");
					mqttClient.publish(http_nodered_latency_topic, String(latency_ms).c_str());
				}

				// Send a string to the client containing "OK"
				String ok = "OK";
				SerialBT.print(ok);
			}
			else if (size > 200)
			{
				SerialBT.flush();
				Serial.println("HTTP Throughput message received");
			}
		}
		free(mqtt_message); // Free the dynamically allocated memory when done

		time_not_connected = millis();
	}

	// delay(100);
}

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

long long compute_latency(String timestamp_received_s, String timestamp_received_us, struct timeval tv)
{

	// Parsing timestamps to integers
	unsigned long timestamp_s = strtoul(timestamp_received_s.c_str(), NULL, 10);
	unsigned long timestamp_us = strtoul(timestamp_received_us.c_str(), NULL, 10);

	// Computing latency
	unsigned long long current_time_ms = tv.tv_sec * 1000ULL + tv.tv_usec / 1000ULL;
	unsigned long long received_time_ms = timestamp_s * 1000ULL + timestamp_us / 1000ULL;
	long long latency_ms = current_time_ms - received_time_ms;

	// Checking for overflow and incorrect timestamps
	if (timestamp_s > tv.tv_sec || (timestamp_s == tv.tv_sec && timestamp_us >= tv.tv_usec))
	{
		Serial.println("Received timestamp is greater than current time, resetting time...");
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		return -1;
	}

	return latency_ms;
}
