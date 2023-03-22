#ifndef MESSAGES_H
#define MESSAGES_H

#include <Arduino.h>


void createMqttMessage(const char* topic, uint8_t* payload, uint16_t payloadSize, uint8_t* mqtt_message);
void createHttpMessage(const char* method, const char* uri, uint8_t* payload, uint16_t payloadLength, uint8_t* http_message);
void createCoapMessage(const char* uri_path, uint8_t* payload, uint16_t payloadLength, uint8_t* coap_message);
void createMessage(const char* method, const char* uri, const char* topic, uint8_t* payload, uint16_t payloadLength, uint8_t* message);

uint16_t computeMqttMessageLength(const char* topic, uint16_t payloadLength);
uint16_t computeHttpMessageLength(const char* method, const char* uri, uint16_t payloadLength);
uint16_t computeCoapMessageLength(const char* uri_path, uint16_t payloadLength);
uint16_t computeMessageLength(const char* type, const char* uri, const char* topic, const char* method, uint16_t payloadLength);


#endif  // MESSAGES_H