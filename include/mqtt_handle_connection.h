#ifndef MQTT_HANDLE_CONNECTION_H
#define MQTT_HANDLE_CONNECTION_H

#include <WiFiClient.h>
#include <PubSubClient.h>
#include <constants.h>

extern const int mqttPort;
extern const char* welcomeTopic;
extern const char* mqttUser;
extern const char* mqttPassword;
extern const char* topic_esp_device_timestamp;
extern const char* topic_throughput;
extern const char* topic_esp_device_message;
extern const char* topic_esp_device_message_ack;
extern const char* topic_nodered_throughput;
extern const char* topic_nodered_packet_loss;
extern const char* topic_nodered_rssi;
extern const char* topic_http_nodered_packet_loss;
extern const char* topic_http_nodered_throughput;
extern const char* topic_coap_nodered_packet_loss;
extern const char* topic_coap_nodered_throughput;
extern int packets_received;
extern const char* mqttServer;



void connectToMQTT(const char* server, int port, const char* user, const char* password, PubSubClient& client);
void subscribeToTopics(PubSubClient& client, const char* welcomeTopic, const char* secondDeviceIpTopic, const char* topic_throughput_ack);
void checkMQTTConnection(PubSubClient& client);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void ip_second_device_callback(char * topic, byte * payload, unsigned int length);
void welcome_callback(char * topic, byte * payload, unsigned int length);
void timestamp_callback(char * topic, byte * payload, unsigned int length);
void message_ack_callback();


#endif