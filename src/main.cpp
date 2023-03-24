#include "wifi_data.h"
#include "wifi_handle_connection.h"
#include "mqtt_handle_connection.h"
#include "http_handle_connection.h"
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <common.h>
#include <coap-simple.h>
#include <WiFiUdp.h>
#include <ESP32Ping.h>

#define ENABLE_MQTT
#define ENABLE_HTTP
#define ENABLE_COAP

const int port = 80;
const int seconds_to_send = 3;

extern bool ipReceived;
extern IPAddress secondDeviceIp;
IPAddress serverIp;
// IPAddress serverIp(192, 168, 154, 28); // Dante's Inferno
// IPAddress serverIp(192, 168, 154, 147); // WIFI2 Dante's Inferno
// IPAddress serverIp(192, 168, 1, 212); // Interstellar
// IPAddress serverIp(192, 168, 1, 40); // TettyPotty5

IPAddress localIp;

WiFiClient wifiClient;
WiFiUDP udp;
Coap coap(udp);

PubSubClient mqttClient(mqttServer, mqttPort, wifiClient);

void printLocalTime();
void publishWelcomeMessage();
void publishLocalIp();
void publishTimestamp();
void coap_callback(CoapPacket &packet, IPAddress ip, int port);

// const char *ntpServer = "ntp1.inrim.it";
const char *ntpServer = "it.pool.ntp.org";
const long gmtOffset_sec = 0;	  /*ECT OFFSET +1 HOURS(3600 SEC)*/
const int daylightOffset_sec = 0; /*1 hour daylight offset*/

const char *coap_latency_topic = "coap/room/devices/esp32-server/latency";
const char *coap_packet_loss_topic = "coap/room/devices/esp32-server/packet-loss";
const char *coap_throughput_topic = "coap/room/devices/esp32-server/throughput";

// char payload[LATENCY_MESSAGE_SIZE];

void setup()
{
	Serial.begin(115200);
	connectToWiFi(ssid, password);
	setLocalIp(localIp);
	Serial.print("Local IP: ");
	Serial.println(localIp);

	// MQTT Connection
	connectToMQTT(mqttServer, mqttPort, mqttUser, mqttPassword, mqttClient);
	subscribeToTopics(mqttClient, welcomeTopic, topicIpSecondDevice, topic_esp_device_message_ack);

	// COAP Configuration
	coap.response(coap_callback);
	coap.start();

	// Time Configuration
	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	printLocalTime();

	// Set its own hostname on esp32 mdns
	MDNS.begin("Esp32-Client");
}

int first_loop = true;

void wait_packets_received(PubSubClient &client, int max_wait_seconds, int packet_received_goal)
{
	unsigned long start_time = millis();

	while (packets_received < packet_received_goal && millis() - start_time < max_wait_seconds * 1000)
	{
		// Wait for packet_received_count to reach the goal or the timeout to expire
		// Check for new packet received and increment packet_received_count if one is received
		client.loop();
	}
}

void wait_packets_coap(Coap &client, int max_wait_seconds, int packet_received_goal)
{
	unsigned long start_time = millis();

	while (packets_received < packet_received_goal && millis() - start_time < max_wait_seconds * 1000)
	{
		// Wait for packet_received_count to reach the goal or the timeout to expire
		// Check for new packet received and increment packet_received_count if one is received
		client.loop();
	}
}

void loop()
{

	// // Get the size of the flash memory
	// uint32_t flash_size = ESP.getFlashChipSize();

	// // Get the size of the free flash memory
	// uint32_t free_flash = ESP.getFreeSketchSpace();

	// // Print the flash memory information
	// Serial.print("Flash size: ");
	// Serial.print(flash_size);
	// Serial.print(", Free space: ");
	// Serial.println(free_flash);

	if (!WiFi.hostByName("Esp32-Server.local", serverIp))
	{
		Serial.println("DNS resolution failed");
		serverIp = MDNS.queryHost("Esp32-Server.local");
		if (serverIp == INADDR_NONE)
		{
			Serial.println("MDNS resolution failed");
			ipReceived = false;
			return;
		}
	}
	ipReceived = true;
	Serial.printf("IP esp32-server: %s\n", serverIp.toString().c_str());
	Ping.ping(serverIp);

	configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
	checkMQTTConnection(mqttClient);
	// need because after HTTP POST request the mqtt connection is kind of closed.
	subscribeToTopics(mqttClient, welcomeTopic, topicIpSecondDevice, topic_esp_device_message_ack);

	if (first_loop)
	{
		mqttClient.publish(welcomeTopic, "hello i'm esp32 first device");
		mqttClient.publish(topic_my_ip, localIp.toString().c_str(), 1);
		first_loop = false;
		return;
	}

	int32_t rssi = WiFi.RSSI();
	mqttClient.publish(topic_nodered_rssi, String(rssi).c_str(), 0);


	if (ipReceived)
	{
#ifdef ENABLE_MQTT
		unsigned int bytes_sent = 0;
		char payload[LATENCY_MESSAGE_SIZE];
		test_latency(payload);
		// mqttClient.publish(topic_esp_device_timestamp, payload);
		// payload[0] = '\0'; // set payload to empty string
		delay(500);
		char message[MESSAGE_SIZE];
		build_large_message(message);

		// Serial.println("Time start: " + String(start_time));

		rssi = WiFi.RSSI();
		mqttClient.publish(topic_nodered_rssi, String(rssi).c_str(), 0);
		unsigned long start_time = millis();

		while (millis() - start_time < seconds_to_send) 
		{
			mqttClient.publish(topic_throughput, message, 0);
			bytes_sent += strlen(message);
			// Wait for the acknowledgement from the broker
			while (mqttClient.loop() != 1)
			{
			}
		}

		unsigned long end_time = millis();
		float throughput = compute_throughput(bytes_sent, start_time, end_time, "MQTT");
		bytes_sent = 0;
		mqttClient.publish(topic_nodered_throughput, String(throughput).c_str(), 0);
		delay(1000);
		int packets_sent = 0;
		const int num_packets = 10;
		packets_received = 0;

		Serial.println("[mqtt]Testing packet loss...");
		for (int i = 0; i < num_packets; i++)
		{
			mqttClient.loop();

			// Increment packets_sent
			packets_sent++;
			char message[50];
			snprintf(message, sizeof(message), "%d", packets_sent);
			mqttClient.publish(topic_esp_device_message, message);
			mqttClient.loop();
			delay(500);
			test_latency(payload);
			mqttClient.publish(topic_esp_device_timestamp, payload);
			delay(500);
		}

		wait_packets_received(mqttClient, 1, num_packets);
		// Print the number of packets sent and received and compute packet loss
		Serial.printf("%d :Packets received \n", packets_received);
		Serial.printf("Packet loss: %.2f%%\n", (packets_sent - packets_received) / (float)packets_sent * 100);
		mqttClient.publish(topic_nodered_packet_loss, String((packets_sent - packets_received) / (float)packets_sent * 100).c_str(), 0);
		packets_sent = 0;
		packets_received = 0;

#endif
#ifdef ENABLE_HTTP
		int http_code = 0;
		HTTPClient httpClient;
		rssi = WiFi.RSSI();
		mqttClient.publish(topic_nodered_rssi, String(rssi).c_str(), 0);
		bool connected_to_http_server = connectToHttpServer(httpClient, serverIp, port, "/latency");
		if (connected_to_http_server)
		{
			throughput = 0.;
			Serial.println("Http-Testing packet loss...");
			for (int i = 0; i < num_packets; i++)
			{
				test_latency(payload);
				http_code = sendHttpPost(httpClient, String(payload));
				packets_sent++;
				if (http_code == HTTP_CODE_OK)
					packets_received++;
				// Serial.println("HTTP POST sent");
				Serial.println("Packets received: " + String(packets_received));
				delay(500);
			}
			closeConnectionToHttpServer(httpClient);

			int packet_loss = (packets_sent - packets_received) / (float)packets_sent * 100;
			Serial.println("HTTP Packet loss: " + String(packet_loss));
			packets_sent = 0;
			packets_received = 0;
			checkMQTTConnection(mqttClient);
			// need because after HTTP POST request the mqtt connection is kind of closed.
			subscribeToTopics(mqttClient, welcomeTopic, topicIpSecondDevice, topic_esp_device_message_ack);
			delay(500);
			mqttClient.publish(topic_http_nodered_packet_loss, String(packet_loss).c_str(), 0);
		}
		else
		{
			Serial.println("Could not connect to HTTP server /latency");
		}

		connected_to_http_server = connectToHttpServer(httpClient, serverIp, port, "/throughput");
		if (connected_to_http_server)
		{
			Serial.println("Http-Testing throughput...");
			bytes_sent = 0;
			start_time = millis();

			// compute throughput sending message for 3 seconds
			while (millis() - start_time < seconds_to_send)
			{
				http_code = sendHttpPost(httpClient, String(message));
				if (http_code == HTTP_CODE_OK)
				{
					bytes_sent += strlen(message);
					// Serial.println("HTTP LARGE MESSAGE POST sent");
				}
			}
			end_time = millis();
			Serial.println("HTTP bytes sent: " + String(bytes_sent));
			throughput = compute_throughput(bytes_sent, start_time, end_time, "HTTP");
			bytes_sent = 0;
			closeConnectionToHttpServer(httpClient);

			// need because after HTTP POST request the mqtt connection is closed.
			checkMQTTConnection(mqttClient);
			subscribeToTopics(mqttClient, welcomeTopic, topicIpSecondDevice, topic_esp_device_message_ack);
			delay(1500);

			mqttClient.publish(topic_http_nodered_throughput, String(throughput).c_str(), 0);
			throughput = 0.;
		}
		else
		{
			Serial.println("Could not connect to HTTP server /throughput");
		}

#endif
#ifdef ENABLE_COAP
		// CoAP server endpoint URL

		// char payload[LATENCY_MESSAGE_SIZE];
		rssi = WiFi.RSSI();
		mqttClient.publish(topic_nodered_rssi, String(rssi).c_str(), 0);
		// test_latency(payload);
		Serial.println("Sending CoAP message");
		int msgid;
		// int msgid = coap.put(serverIp, 5683, "latency", payload);
		// coap.loop();
		// int msgid = coap.get(serverIp, 5683, "/latency");
		packets_sent = 0;
		delay(1000);

		for (int i = 0; i < num_packets; i++)
		{
			test_latency(payload);
			Serial.println("Sending CoAP message");
			msgid = coap.put(serverIp, 5683, coap_latency_topic, payload);
			coap.loop();
			delay(500);
			coap.put(serverIp, 5683, coap_packet_loss_topic, "Ping");
			packets_sent++;
			coap.loop();
			delay(500);
		}
		wait_packets_coap(coap, 1, num_packets);
		delay(1000);

		Serial.println("CoAP packet loss: " + String((num_packets - packets_received) / (float)num_packets * 100) + "%");
		checkMQTTConnection(mqttClient);
		subscribeToTopics(mqttClient, welcomeTopic, topicIpSecondDevice, topic_esp_device_message_ack);
		delay(500);
		mqttClient.publish(topic_coap_nodered_packet_loss, String((num_packets - packets_received) / (float)num_packets * 100).c_str(), 0);
		packets_received = 0;
		// send a large message

		Serial.println("Sending CoAP large message");
		start_time = millis();
		while(millis() - start_time < seconds_to_send)
		{
			coap.put(serverIp, 5683, coap_throughput_topic, message);
			bytes_sent += strlen(message);
			coap.loop();
		}
		end_time = millis();
		throughput = compute_throughput(bytes_sent, start_time, end_time, "CoAP");
		bytes_sent = 0;
		mqttClient.publish(topic_coap_nodered_throughput, String(throughput).c_str(), 0);
		delay(1000);

#endif
	}

	delay(1000);
}

void printLocalTime()
{
	struct tm timeinfo;

	unsigned long last_sync_time= millis();
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

void publishWelcomeMessage()
{
	mqttClient.publish(welcomeTopic, "hello i'm esp32 first device");
}

void publishLocalIp()
{
	mqttClient.publish(topic_my_ip, localIp.toString().c_str(), 1);
}

void publishTimestamp()

{
	char payload[64]; // 32 for the 'A's and 32 for the timestamp

	// Fill the payload with 'A's
	for (int i = 0; i < 32; i++)
	{
		payload[i] = 'A';
	}

	// Add the timestamp to the end of the payload
	struct timeval tv;
	gettimeofday(&tv, NULL);
	snprintf(payload + 32, sizeof(payload) - 32, "%ld.%06ld", tv.tv_sec, tv.tv_usec);

	// Publish the payload
	mqttClient.publish(topic_esp_device_timestamp, payload);
	payload[0] = '\0'; // set payload to empty string
	printf("Sec. %ld, usec. %ld", tv.tv_sec, tv.tv_usec);
}

// CoAP client response callback
void coap_callback(CoapPacket &packet, IPAddress ip, int port)
{
	Serial.println("[CoAP Response received]");

	char p[packet.payloadlen + 1];
	memcpy(p, packet.payload, packet.payloadlen);
	p[packet.payloadlen] = '\0';

	Serial.println("message: " + String(p));
	if (strcmp(p, "Pong") == 0)
	{
		packets_received++;
	}
}
