#include "messages.h"

void createMqttMessage(const char* topic, uint8_t* payload, uint16_t payloadLength, uint8_t* mqtt_message) {
    // MQTT Header
    uint16_t i = 1;
    mqtt_message[0] = 'M'; // Type identifier for MQTT
    mqtt_message[i++] = 0x30;  // Fixed header - 0x30: The MQTT message type and flags. This value represents
                                // a publish message with QoS 0 and no retained message.

    // Remaining length
    uint16_t remainingLength = 2 + strlen(topic) + payloadLength;
    if (payloadLength % 4 != 0) {
        remainingLength += 4 - (payloadLength % 4);
    }
    mqtt_message[i++] = remainingLength >> 8;
    mqtt_message[i++] = remainingLength & 0xFF;

    // Topic length
    uint16_t topicLength = strlen(topic);
    mqtt_message[i++] = topicLength >> 8;
    mqtt_message[i++] = topicLength & 0xFF;

    // Topic
    strcpy((char*)&mqtt_message[i], topic);
    i += topicLength;

    // Padding
    while (i % 4 != 0) {
        mqtt_message[i++] = 0x00;
    }

    // Payload
    memcpy(&mqtt_message[i], payload, payloadLength);
    i += payloadLength;

    // Add null terminator
    mqtt_message[i] = '\n';
}



void createHttpMessage(const char* method, const char* uri, uint8_t* payload, uint16_t payloadLength, uint8_t* http_message) {
    // HTTP Request Line
    uint16_t i = 1;
    http_message[0] = 'H'; // Type identifier for HTTP
    i += sprintf((char*)&http_message[i], "%s %s HTTP/1.1\r\n", method, uri);

    // HTTP Headers
    i += sprintf((char*)&http_message[i], "Content-Length: %d\r\n", payloadLength);
    i += sprintf((char*)&http_message[i], "Connection: close\r\n");
    i += sprintf((char*)&http_message[i], "Content-Type: application/x-www-form-urlencoded\r\n");
    i += sprintf((char*)&http_message[i], "\r\n");

    // Payload
    memcpy(&http_message[i], payload, payloadLength);
    i += payloadLength;

    // Add terminator
    http_message[i] = '\n';
}

void createCoapMessage(const char* uri_path, uint8_t* payload, uint16_t payloadLength, uint8_t* coap_message) {
    // CoAP Header
    uint16_t i = 1;
    coap_message[0] = 'C'; // Type identifier for CoAP
    coap_message[i] = 0x44; // 0b01000100 - version=1, type=Non-confirmable (0), token length=0, code=GET(0.01), message ID=0
    i += 2;

    // URI Path
    uint16_t pathLength = strlen(uri_path);
    memcpy(&coap_message[i], uri_path, pathLength);
    i += pathLength;

    // Payload Marker
    coap_message[i++] = 0xFF;

    // Payload
    memcpy(&coap_message[i], payload, payloadLength);
    i += payloadLength;

    // Add terminator
    coap_message[i] = '\n';
}



void createMessage(const char* method,const char* uri, const char* topic, uint8_t* payload, uint16_t payloadLength, uint8_t* message) {
    if (strcmp(method, "HTTP") == 0) {
        createHttpMessage(method, uri, payload, payloadLength, message);
    } else if (strcmp(method, "MQTT") == 0) {
        createMqttMessage(topic, payload, payloadLength, message);
    } else if (strcmp(method, "COAP") == 0) {
        createCoapMessage(uri, payload, payloadLength, message);
    }
}



uint16_t computeMqttMessageLength(const char* topic, uint16_t payloadLength) {
    uint16_t topicLength = strlen(topic);
    uint16_t remainingLength = 2 + topicLength + payloadLength;
    if (payloadLength % 4 != 0) {
        remainingLength += 4 - (payloadLength % 4);
    }
    return 1 + remainingLength + 1; // 1 for type identifier, 1 for terminator character
}

uint16_t computeHttpMessageLength(const char* method, const char* uri, uint16_t payloadLength) {
    uint16_t headersLength = strlen("Content-Length: \r\nConnection: close\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n");
    uint16_t requestLineLength = strlen(method) + strlen(uri) + strlen(" HTTP/1.1\r\n");
    uint8_t payloadLengthStr[5];
    sprintf((char*)payloadLengthStr, "%u", payloadLength);
    headersLength += strlen((char*)payloadLengthStr); // Add the length of the payload length string to the headers length
    uint16_t messageLength = requestLineLength + headersLength + payloadLength;
    return messageLength + 2; // Add 2 bytes for the type identifier and terminator character
}




uint16_t computeCoapMessageLength(const char* uri_path, uint16_t payloadLength) {
    uint16_t remainingLength = strlen(uri_path) + 6 + payloadLength;
    return 1 + remainingLength + 1; // 1 for type identifier, 1 for terminator character
}


uint16_t computeMessageLength(const char* type, const char* uri, const char* topic, const char* method, uint16_t payloadLength) {
    if (strcmp(type, "MQTT") == 0) {
        return computeMqttMessageLength(topic, payloadLength);
    } else if (strcmp(type, "COAP") == 0) {
        return computeCoapMessageLength(uri, payloadLength);
    } else if (strcmp(type, "HTTP") == 0) {
        return computeHttpMessageLength(method, uri, payloadLength);
    } else {
        return 0;
    }
}