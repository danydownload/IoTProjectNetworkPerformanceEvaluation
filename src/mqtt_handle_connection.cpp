#include "mqtt_handle_connection.h"

// const char *mqttServer = "192.168.154.28"; // dante's inferno
// const char *mqttServer = "192.168.154.147"; // WIFI 2 dante's inferno
// const char *mqttServer = "192.168.1.40"; //tettypotty5
// const char *mqttServer = "192.168.1.212"; //interstellar5
const char *mqttServer = "MSI.local";
const int mqttPort = 1883;
const char* welcomeTopic = "welcome";
const char* topicIpSecondDevice = "room/devices/pc_device/ip_pc_device";
const char* topic_my_ip = "room/devices/esp_device/ip_esp_device";
const char* topic_esp_device_timestamp = "room/devices/esp_device/timestamp";
const char* topic_throughput = "room/devices/esp_device/throughput";
const char* topic_esp_device_message = "room/devices/esp_device/message";
const char* topic_esp_device_message_ack = "room/devices/esp_device/message/ack";
const char* topic_nodered_throughput = "nodered/throughput";
const char* topic_nodered_packet_loss = "nodered/packet_loss";
const char* topic_nodered_rssi = "nodered/rssi";
const char* topic_http_nodered_packet_loss = "nodered/http/packet_loss";
const char* topic_http_nodered_throughput = "nodered/http/throughput";
const char* topic_coap_nodered_packet_loss = "nodered/coap/packet_loss";
const char* topic_coap_nodered_throughput = "nodered/coap/throughput";

const char* mqttUser = "";
const char* mqttPassword = "";

bool ipReceived = false;
IPAddress secondDeviceIp;
int packets_received = 0;


void connectToMQTT(const char* server, int port, const char* user, const char* password, PubSubClient& client) {
  WiFiClient wifiClient;
  client.setServer(server, port);
  client.setCallback(mqttCallback);
  while (!client.connected()) {
    if (client.connect("ESP32Client-1", user, password)) {
      Serial.println("Connected to MQTT broker");
    } else {
      Serial.println("Connection to MQTT broker failed. Trying again in 5 seconds");
      delay(5000);
    }
  }
}

void subscribeToTopics(PubSubClient& client, const char* welcomeTopic, const char* secondDeviceIpTopic, const char* topic_esp_device_message_ack) {
  // print topic to subscribe to
  Serial.print("Subscribing to topic: ");
  Serial.println(welcomeTopic);
  client.subscribe(welcomeTopic);
  
  Serial.print("Subscribing to topic: ");
  Serial.println(secondDeviceIpTopic);
  client.subscribe(secondDeviceIpTopic);

  Serial.print("Subscribing to topic: ");
  Serial.println(topic_esp_device_message_ack);
  client.subscribe(topic_esp_device_message_ack);
}


void checkMQTTConnection(PubSubClient& client) {
  if (!client.connected()) {
    client.connect("ESP32Client-1");
    Serial.println("MQTT connected");
  }
  else {
    Serial.println("MQTT already connected");
  }
  client.loop();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // printf("Message arrived on topic: [%s] \n", topic);
  if (strcmp (topic, welcomeTopic) == 0)
	{
		Serial.println("Welcome message received");
		welcome_callback(topic, payload, length);
	}
	else if (strcmp (topic, topicIpSecondDevice) == 0)
	{
		// Serial.println("IP second device received");
		ip_second_device_callback(topic, payload, length);
    
	}
  else if (strcmp (topic, topic_esp_device_message_ack) == 0)
  {
    // Serial.println("Throughput ack received");
    message_ack_callback();
  }
}

void welcome_callback(char * topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}


void ip_second_device_callback(char * topic, byte * payload, unsigned int length) {
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
	String message = "";
	for (int i = 0; i < length; i++) {
		message += (char)payload[i];
	}
	// Serial.println("Received ip: " + message);

  secondDeviceIp.fromString(message);
	// Serial.print("IP address: ");
	// Serial.println(secondDeviceIp);

	ipReceived = true;
}


void timestamp_callback(char * topic, byte * payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void message_ack_callback()
{
  // Serial.println("\nThroughput ack received");
  packets_received++;
  Serial.println("Packets received: " + String(packets_received));
}