#include "mqtt_handle_connection.h"
#include "wifi_handle_connection.h"

void connectToMQTT(const char *server, int port)
{
    WiFiClient wifiClient;
    mqttClient.setServer(server, port);
    mqttClient.setCallback(mqttCallback);
    while (!mqttClient.connected())
    {
        if (mqttClient.connect("ESP32Client-1"))
        {
            Serial.println("Connected to MQTT broker");
        }
        else
        {
            Serial.println("Connection to MQTT broker failed. Trying again in 5 seconds");
            delay(5000);
        }
    }
}

void checkMQTTConnection()
{
    if (!mqttClient.connected())
    {
        mqttClient.connect("ESP32Client-1");
        Serial.println("MQTT connected");
    }
    // else
    // {
    //     Serial.println("MQTT already connected");
    // }
    mqttClient.loop();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    printf("Message arrived on topic: [%s] \n", topic);
}
