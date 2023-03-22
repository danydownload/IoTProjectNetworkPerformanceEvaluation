#include "mqtt_handle_connection.h"
#include "constants.h"
#include "utils.h"
#include "wifi_handle_connection.h"

const char *mqttServer = "MSI.local";
const int mqttPort = 1883;
const char *welcomeTopic = "welcome";
const char *topicIpFirstDevice = "room/devices/first_device/ip_first_device";
const char *topicMyIp = "room/devices/second_device/ip_second_device";
const char *topic_esp_device_timestamp = "room/devices/esp_device/timestamp";
const char *mqttUser = "";
const char *mqttPassword = "";
const char *topic_esp_device_message = "room/devices/esp_device/message";
const char *topic_esp_device_message_ack = "room/devices/esp_device/message/ack";
const char* topic_throughput = "room/devices/esp_device/throughput";
const char *topic_nodered_mqtt_latency = "nodered/mqtt/latency";
const char *topic_nodered_http_latency = "nodered/http/latency";
const char *topic_nodered_coap_latency = "nodered/coap/latency";

unsigned long last_mqtt_message_received = 0;

PubSubClient mqttClient(wifiClient);


void connectToMQTT(const char *server, int port, const char *user, const char *password, PubSubClient &client)
{
    WiFiClient wifiClient;
    client.setServer(server, port);
    client.setCallback(mqttCallback);
    while (!client.connected())
    {
        if (client.connect("ESP32Server", user, password))
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

void subscribeToTopics()
{
    Serial.print("Subscribing to topic: ");
    Serial.println(topic_esp_device_timestamp);
    mqttClient.subscribe(topic_esp_device_timestamp);

    Serial.print("Subscribing to topic: ");
    Serial.println(topic_esp_device_message);
    mqttClient.subscribe(topic_esp_device_message);
}

void checkMQTTConnection(PubSubClient &client)
{
    if (!client.connected())
    {
        client.connect("ESP32Client-1");
        Serial.println("MQTT connected");
    }
    client.loop();
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    // Computing latency
    struct timeval tv;
    gettimeofday(&tv, NULL);

    last_mqtt_message_received = millis();

    String message = "";
    for (int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    if (strcmp(topic, topic_esp_device_timestamp) == 0)
    {
        long long latency_ms = compute_latency(message, tv);
        if (latency_ms > 0)
        {
            // Serial.print("MQTT Latency (ms): ");
            // Serial.println(latency_ms);
            mqttClient.publish(topic_nodered_mqtt_latency, String(latency_ms).c_str());
        }
    }
    else if (strcmp(topic, topic_esp_device_message) == 0)
    {
        Serial.println("Received message from first device: " + message);
        mqttClient.publish(topic_esp_device_message_ack, "ACK");
    }

}