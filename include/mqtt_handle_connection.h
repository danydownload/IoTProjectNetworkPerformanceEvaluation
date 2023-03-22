#ifndef MQTT_HANDLE_CONNECTION_H
#define MQTT_HANDLE_CONNECTION_H

#include <WiFiClient.h>
#include <PubSubClient.h>

extern const char *mqttServer;
extern const int mqttPort;
extern const char *welcomeTopic;
extern const char *topicIpFirstDevice;
extern const char *topicMyIp;
extern const char *topic_esp_device_timestamp;
extern const char *mqttUser;
extern const char *mqttPassword;
extern const char *topic_esp_device_message;
extern const char *topic_esp_device_message_ack;
extern const char *topic_throughput;
extern const char *topic_nodered_mqtt_latency;
extern const char *topic_nodered_http_latency;
extern const char *topic_nodered_coap_latency;
extern unsigned long last_mqtt_message_received;

extern PubSubClient mqttClient;


void connectToMQTT(const char* server, int port, const char* user, const char* password, PubSubClient& client);
void subscribeToTopics();
void checkMQTTConnection(PubSubClient& client);
void mqttCallback(char* topic, byte* payload, unsigned int length);

/*
void compute_latency(String message, struct timeval tv);
*/

// void ip_second_device_callback(char * topic, byte * payload, unsigned int length);
void welcome_callback(char * topic, byte * payload, unsigned int length);

void timestamp_callback(char * topic, byte * payload, unsigned int length);
void message_ack_callback();


#endif