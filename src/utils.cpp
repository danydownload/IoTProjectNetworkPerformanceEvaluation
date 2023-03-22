#include "utils.h"
#include "messages.h"
#include "mqtt_handle_connection.h"

const int MESSAGE_SIZE = 512;
// const int MESSAGE_SIZE = 80;

void build_large_message(char *message)
{
	// send a large message
	for (int i = 0; i < MESSAGE_SIZE; i++)
	{
		message[i] = 'a' + i % 26;
	}
	message[MESSAGE_SIZE - 1] = '\0';
}

void compute_packet_loss_and_latency_and_throughput(const char *type, const char *uri, const char *topic, const char *method)
{
	int message_sents = 0;
	int message_received = 0;

	for (int i = 0; i < 10; i++)
	{
		configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
		struct timeval tv;
		gettimeofday(&tv, NULL);
		char timestamp[18];
		// Serial.println("tv_usec: " + String(tv.tv_usec) + " tv_sec: " + String(tv.tv_sec));
		snprintf(timestamp, sizeof(timestamp), "%ld.%06ld", tv.tv_sec, tv.tv_usec);
		// Serial.println("Timestamp: " + String(timestamp));


		uint8_t payload[] = {
			'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
			'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
			'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
			'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
			timestamp[0], timestamp[1], timestamp[2], timestamp[3], timestamp[4], timestamp[5],
			timestamp[6], timestamp[7], timestamp[8], timestamp[9], timestamp[10], timestamp[11],
			timestamp[12], timestamp[13], timestamp[14], timestamp[15], timestamp[16], timestamp[17]};
		uint16_t payloadSize = sizeof(payload);

		// uint16_t mqtt_message_length = computeMqttMessageLength(topic, payloadSize);
		uint16_t mqtt_message_length = computeMessageLength(type, uri, topic, method, payloadSize);
		Serial.println("Size of the " + String(type) + " message: " + String(mqtt_message_length));


		// 1 for type of message(M->MQTT, C->CoAP, H->HTTP), 14 header, payloadSize, 1 for terminator character '\n'
		uint8_t mqtt_message[mqtt_message_length];

		gettimeofday(&tv, NULL);
		snprintf(timestamp, sizeof(timestamp), "%ld.%06ld", tv.tv_sec, tv.tv_usec);
		// Replace the last 18 bytes of payload with timestamp
		memcpy(payload + payloadSize - 18, timestamp, 18);

		createMessage(type, uri, topic, payload, payloadSize, mqtt_message);

		// Serial.println("Size of mqtt_message: " + String(sizeof(mqtt_message)));

		SerialBT.println(mqtt_message_length);

		// The SerialBT.write() function can send up to 256 bytes of data in a single call, which is equivalent to the maximum value of an uint8_t
		SerialBT.write(mqtt_message, sizeof(mqtt_message));
		message_sents++;

		// Attendi 5 secondi per ottenere risposta
		unsigned long start_time = millis();
		while (millis() - start_time < 5000)
		{
			String ok = SerialBT.readString();
			if (ok == "OK")
			{
				// Serial.println("Received OK");
				message_received++;
				break;
			}
		}

	}

	// Print statistics
	Serial.println("Message sents: " + String(message_sents));
	Serial.println("Message received: " + String(message_received));
	Serial.println("Message lost: " + String((message_sents - message_received) * 100 / message_sents) + "%");
	float packet_loss = (message_sents - message_received) * 100 / message_sents;

	checkMQTTConnection();

	if (type[0] == 'M')
	{
		Serial.println("Publishing packet loss to MQTT topic: " + String(mqtt_nodered_packet_loss_topic) + " with value: " + String(packet_loss));
		mqttClient.publish(mqtt_nodered_packet_loss_topic, String(packet_loss).c_str());
	}
	else if (type[0] == 'C')
	{
		Serial.println("Publishing packet loss to CoAP topic: " + String(coap_nodered_packet_loss_topic) + " with value: " + String(packet_loss));
		mqttClient.publish(coap_nodered_packet_loss_topic, String(packet_loss).c_str());
	}
	else if (type[0] == 'H')
	{
		Serial.println("Publishing packet loss to HTTP topic: " + String(http_nodered_packet_loss_topic) + " with value: " + String(packet_loss));
		mqttClient.publish(http_nodered_packet_loss_topic, String(packet_loss).c_str());
	}

	// Compute throughput
	// 1 for type of message(M->MQTT, C->CoAP, H->HTTP), 14 header, payloadSize, 1 for terminator character '\n'

	uint8_t payloadSize = 100;
	uint8_t payload[payloadSize];

	for (int i = 0; i < payloadSize; i++)
	{
		payload[i] = 'a' + i % 26;
	}

	uint8_t heavy_message_length = computeMessageLength(type, uri, topic, method, payloadSize);
	uint8_t heavy_message[heavy_message_length];
	Serial.println("Heavy message length: " + String(heavy_message_length));
	createMessage(type, uri, topic, payload, payloadSize, heavy_message);
	SerialBT.println(heavy_message_length);

	unsigned long byte_sents = 0;
	unsigned long start_time = millis();
	int count = 0;
	while (millis() - start_time < 1000)
	{
		count++;
		SerialBT.write(heavy_message, sizeof(heavy_message));
		byte_sents += sizeof(heavy_message);
		delay(20);
	}
	unsigned long end_time = millis();
	float time_taken = (end_time - start_time) / 1000.0; // ms to s
	// print the throughput
	Serial.println("Throughput: " + String(byte_sents / time_taken) + " bytes/s");
	Serial.println("Throughput: " + String(byte_sents / time_taken * 8 / 1024) + " kbps");
	Serial.println("Throughput: " + String(byte_sents / time_taken * 8 / 1024 / 1024) + " mbps");
	float throughput_b_s = byte_sents / time_taken;

	checkMQTTConnection();
	if (type[0] == 'M')
	{
		Serial.println("Publishing throughput to MQTT topic: " + String(mqtt_nodered_throughput_topic) + " with value: " + String(throughput_b_s));
		mqttClient.publish(mqtt_nodered_throughput_topic, String(throughput_b_s).c_str());
	}
	else if (type[0] == 'C')
	{
		Serial.println("Publishing throughput to CoAP topic: " + String(coap_nodered_throughput_topic) + " with value: " + String(throughput_b_s));
		mqttClient.publish(coap_nodered_throughput_topic, String(throughput_b_s).c_str());
	}
	else if (type[0] == 'H')
	{
		Serial.println("Publishing throughput to HTTP topic: " + String(http_nodered_throughput_topic) + " with value: " + String(throughput_b_s));
		mqttClient.publish(http_nodered_throughput_topic, String(throughput_b_s).c_str());
	}

	delay(1000);

}