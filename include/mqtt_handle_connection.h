#ifndef MQTT_HANDLE_CONNECTION_H
#define MQTT_HANDLE_CONNECTION_H

#include <PubSubClient.h>

extern PubSubClient mqttClient;
extern const char *mqtt_nodered_latency_topic;
extern const char *coap_nodered_latency_topic;
extern const char *http_nodered_latency_topic;



void connectToMQTT(const char* server, int port);
void checkMQTTConnection();
void mqttCallback(char* topic, byte* payload, unsigned int length);

#endif